// Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5.
// See LICENSE for details.

#include "SMCRSCard.hpp"
#include "AMDCommon.hpp"
#include <Headers/kern_iokit.hpp>

OSDefineMetaClassAndStructors(SMCRSCard, OSObject);

IOReturn SMCRSCard::ensureRMMIOMapped() {
    if (this->rmmio == nullptr) {
        IOLockLock(this->regLock);
        this->dev->setMemoryEnable(true);
        this->dev->setBusMasterEnable(true);
        auto bar5 = this->attributes.getFamily() >= SMCRSChipFamily::SouthernIslands;
        this->rmmio =
            this->dev->mapDeviceMemoryWithRegister(bar5 ? kIOPCIConfigBaseAddress5 : kIOPCIConfigBaseAddress2);
        if (this->rmmio == nullptr || !this->rmmio->getLength()) {
            SYSLOG("RSCard", "Failed to map BAR%d", bar5 ? 5 : 2);
            OSSafeReleaseNULL(this->rmmio);
            IOLockUnlock(this->regLock);
            return kIOReturnDeviceError;
        }
        this->rmmioPtr = reinterpret_cast<volatile UInt32 *>(this->rmmio->getVirtualAddress());
        if (this->rmmioPtr == nullptr) {
            SYSLOG("RSCard", "Failed to get virtual address for BAR%d", bar5 ? 5 : 2);
            OSSafeReleaseNULL(this->rmmio);
            IOLockUnlock(this->regLock);
            return kIOReturnDeviceError;
        }
        DBGLOG("RSCard", "Using BAR%d, located at %p", bar5 ? 5 : 2, this->rmmioPtr);
        IOLockUnlock(this->regLock);
    }

    return kIOReturnSuccess;
}

UInt32 SMCRSCard::readReg32(UInt32 reg) {
    IOLockLock(this->regLock);
    UInt32 ret;
    if ((reg * sizeof(*this->rmmioPtr)) < this->rmmio->getLength()) {
        ret = this->rmmioPtr[reg];
    } else {
        this->rmmioPtr[mmPCIE_INDEX] = reg;
        ret = this->rmmioPtr[mmPCIE_DATA];
    }
    IOLockUnlock(this->regLock);
    return ret;
}

void SMCRSCard::writeReg32(UInt32 reg, UInt32 val) {
    IOLockLock(this->regLock);
    if ((reg * sizeof(*this->rmmioPtr)) < this->rmmio->getLength()) {
        this->rmmioPtr[reg] = val;
    } else {
        this->rmmioPtr[mmPCIE_INDEX] = reg;
        this->rmmioPtr[mmPCIE_DATA] = val;
    }
    IOLockUnlock(this->regLock);
}

UInt32 SMCRSCard::soc15ReadReg32(UInt32 reg) {
    IOLockLock(this->regLock);
    UInt32 ret;
    if ((reg * sizeof(*this->rmmioPtr)) < this->rmmio->getLength()) {
        ret = this->rmmioPtr[reg];
    } else {
        this->rmmioPtr[mmPCIE_INDEX2] = reg;
        ret = this->rmmioPtr[mmPCIE_DATA2];
    }
    IOLockUnlock(this->regLock);
    return ret;
}

void SMCRSCard::soc15WriteReg32(UInt32 reg, UInt32 val) {
    IOLockLock(this->regLock);
    if ((reg * sizeof(*this->rmmioPtr)) < this->rmmio->getLength()) {
        this->rmmioPtr[reg] = val;
    } else {
        this->rmmioPtr[mmPCIE_INDEX2] = reg;
        this->rmmioPtr[mmPCIE_DATA2] = val;
    }
    IOLockUnlock(this->regLock);
}

UInt32 SMCRSCard::readIndirectSMC(UInt32 reg) {
    IOLockLock(this->indLock);
    UInt32 ret;
    switch (this->attributes.getFamily()) {
        case SMCRSChipFamily::SeaIslands:
        case SMCRSChipFamily::SouthernIslands:
            this->writeReg32(mmSMC_IND_INDEX_0, reg);
            ret = this->readReg32(mmSMC_IND_DATA_0);
            break;
        case SMCRSChipFamily::VolcanicIslands:
            this->writeReg32(mmSMC_IND_INDEX_11, reg);
            ret = this->readReg32(mmSMC_IND_DATA_11);
            break;
        default:
            ret = 0xFFFFFFFF;
            break;
    }
    IOLockUnlock(this->indLock);
    return ret;
}

void SMCRSCard::writeIndirectSMC(UInt32 reg, UInt32 val) {
    IOLockLock(this->indLock);
    switch (this->attributes.getFamily()) {
        case SMCRSChipFamily::SeaIslands:
        case SMCRSChipFamily::SouthernIslands:
            this->writeReg32(mmSMC_IND_INDEX_0, reg);
            this->writeReg32(mmSMC_IND_DATA_0, val);
            break;
        case SMCRSChipFamily::VolcanicIslands:
            this->writeReg32(mmSMC_IND_INDEX_11, reg);
            this->writeReg32(mmSMC_IND_DATA_11, val);
            break;
        default:
            break;
    }
    IOLockUnlock(this->indLock);
}

UInt32 SMCRSCard::smu7WaitForSMCResp() {
    UInt32 ret = 0;
    for (UInt32 i = 0; i < AMDGPU_MAX_USEC_TIMEOUT; i++) {
        ret = this->readReg32(mmSMC_RESP_0_SMU7) & SMC_RESP_0_MASK_SMU7;
        if (ret != 0) { break; }

        IOSleep(1);
    }

    return ret;
}

UInt32 SMCRSCard::smu7SendMessageToSMC(UInt32 message, UInt32 param, UInt32 *outData) {
    IOLockLock(this->smcLock);
    this->smu7WaitForSMCResp();
    this->writeReg32(mmSMC_MSG_ARG_0_SMU7, param);
    this->writeReg32(mmSMC_RESP_0_SMU7, 0);
    this->writeReg32(mmSMC_MESSAGE_0_SMU7, message);

    const auto resp = this->smu7WaitForSMCResp();

    if (outData != nullptr) { *outData = this->readReg32(mmSMC_MSG_ARG_0_SMU7); }

    IOLockUnlock(this->smcLock);
    return resp;
}

UInt32 SMCRSCard::smu9WaitForSMCResp() {
    UInt32 ret = 0;
    for (UInt32 i = 0; i < AMDGPU_MAX_USEC_TIMEOUT; i++) {
        ret = this->readReg32(MP_BASE_SMU9 + mmMP1_SMN_C2PMSG_90);
        if (ret != 0) { break; }

        IOSleep(1);
    }

    return ret;
}

UInt32 SMCRSCard::smu9SendMessageToSMC(UInt32 message, UInt32 param, UInt32 *outData) {
    IOLockLock(this->smcLock);
    this->smu7WaitForSMCResp();
    this->writeReg32(MP_BASE_SMU9 + mmMP1_SMN_C2PMSG_82, param);
    this->writeReg32(MP_BASE_SMU9 + mmMP1_SMN_C2PMSG_90, 0);
    this->writeReg32(MP_BASE_SMU9 + mmMP1_SMN_C2PMSG_66, message);

    const auto resp = this->smu7WaitForSMCResp();

    if (outData != nullptr) { *outData = this->readReg32(mmMP1_SMN_C2PMSG_82); }

    IOLockUnlock(this->smcLock);
    return resp;
}

IOReturn SMCRSCard::smu7GetTemp(UInt16 *data) {
    auto ctfTemp = GET_THERMAL_STATUS_CTF_TEMP(this->readIndirectSMC(ixCG_MULT_THERMAL_STATUS));
    *data = (ctfTemp & 0x200) ? 255 : (ctfTemp & 0x1FF);
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::thm9GetTemp(UInt16 *data) {
    auto reg = this->soc15ReadReg32(THM_BASE + mmCG_MULT_THERMAL_STATUS_THM9);
    *data = GET_THERMAL_STATUS_CTF_TEMP(reg) & 0x1FF;
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::thm11GetTemp(UInt16 *data) {
    auto reg = this->soc15ReadReg32(THM_BASE + mmCG_MULT_THERMAL_STATUS_THM11);
    *data = GET_THERMAL_STATUS_CTF_TEMP(reg) & 0x1FF;
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::thm10GetTemp(UInt16 *data) {
    auto reg = this->soc15ReadReg32(THM_BASE + mmTHM_TCON_CUR_TMP);
    *data = GET_TCON_CUR_TEMP(reg) / 8;
    if (reg & CUR_TEMP_RANGE_SEL) { *data -= 49; }
    return kIOReturnSuccess;
}

static inline float fractional8BitToFloat(UInt32 value) {
    return static_cast<float>((value & 0xFFFFFF00) >> 8) + static_cast<float>(value & 0xFF) * 0.001;
}

IOReturn SMCRSCard::smu7GetPowerPMStatus(float *data) {
    this->smu7SendMessageToSMC(PPSMC_MSG_PmStatusLogStart_SMU7);
    this->writeIndirectSMC(ixSMU_PM_STATUS_95, 0);
    UInt32 value = 0;
    for (size_t i = 0; i < 10; i += 1) {
        this->smu7SendMessageToSMC(PPSMC_MSG_PmStatusLogSample_SMU7);
        value = this->readIndirectSMC(ixSMU_PM_STATUS_95);
        if (value != 0) { break; }
        IOSleep(500);
    }
    if (value == 0) { return kIOReturnError; }
    *data = fractional8BitToFloat(value);
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::smu7GetPowerSMC(float *data) {
    UInt32 value = 0;
    if (this->smu7SendMessageToSMC(PPSMC_MSG_GetCurrPkgPwr_SMU7, 0, &value) != 1) { return kIOReturnError; }
    if (value == 0) { return this->smu7GetPowerPMStatus(data); }
    *data = fractional8BitToFloat(value);
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::smu9GetPower(float *data) {
    UInt32 value = 0;
    if (this->smu9SendMessageToSMC(PPSMC_MSG_GetCurrPkgPwr_SMU9, 0, &value) != 1) { return kIOReturnError; }
    *data = static_cast<float>(value);
    return kIOReturnSuccess;
}

bool SMCRSCard::initialise(IOPCIDevice *device) {
    if (device == nullptr) { return false; }

    this->dev = device;

    auto deviceID = WIOKit::readPCIConfigValue(this->dev, WIOKit::kIOPCIConfigDeviceID);
    switch (deviceID) {
        case 0x15D8:    // Picasso
        case 0x15DD:    // Raven
        case 0x15E7:    // Barcelo
        case 0x1636:    // Renoir
        case 0x1638:    // Cezanne
        case 0x164C:    // Lucienne
            this->attributes.setFamily(SMCRSChipFamily::Raven);
            break;
        case 0x66A0 ... 0x66AF:    // Vega 20
            this->attributes.setFamily(SMCRSChipFamily::ArcticIslands);
            this->attributes.setIsTHM11();
            break;
        case 0x6860 ... 0x687F:    // Vega 10
            this->attributes.setSupportsPower();
        case 0x69A0 ... 0x69AF:    // Vega 12
            this->attributes.setFamily(SMCRSChipFamily::ArcticIslands);
            break;
        case 0x67C0 ... 0x67FF:    // Polaris 10, Polaris 11
        case 0x6980 ... 0x699F:    // Polaris 12
            this->attributes.setFamily(SMCRSChipFamily::VolcanicIslands);
            this->attributes.setSupportsPower();
            break;
        case 0x6600 ... 0x666F:    // Bonaire
        case 0x67A0 ... 0x67BF:    // Hawaii
        case 0x6900 ... 0x693F:    // Tonga
            this->attributes.setFamily(SMCRSChipFamily::SouthernIslands);
            this->attributes.setSupportsPower();
            break;
        case 0x6780 ... 0x679F:    // Tahiti
        case 0x6800 ... 0x683F:    // Venus, Cape Verde, ...
            this->attributes.setFamily(SMCRSChipFamily::SeaIslands);
            this->attributes.setSupportsPower();
            break;
        case 0x7310 ... 0x73FF:    // Navi 1x, Navi 2x
            this->attributes.setFamily(SMCRSChipFamily::Navi);
            break;
        default:
            SYSLOG("RSCard", "Unsupported card 0x%04X", deviceID);
            return false;
    }

    DBGLOG("RSCard", "Chip Family: %s", stringifyChipFamily(this->attributes.getFamily()));
    DBGLOG("RSCard", "Is THM11: %s", this->attributes.isTHM11() ? "Yes" : "No");
    DBGLOG("RSCard", "Power: %s", this->attributes.supportsPower() ? "Supported" : "Unsupported");

    this->regLock = IOLockAlloc();
    if (regLock == nullptr) {
        SYSLOG("RSCard", "Failed to allocate regLock");
        return false;
    }

    this->indLock = IOLockAlloc();
    if (indLock == nullptr) {
        SYSLOG("RSCard", "Failed to allocate indLock");
        return false;
    }

    this->smcLock = IOLockAlloc();
    if (smcLock == nullptr) {
        SYSLOG("RSCard", "Failed to allocate smcLock");
        return false;
    }

    return true;
}

IOReturn SMCRSCard::getTemperature(UInt16 *data) {
    auto ret = this->ensureRMMIOMapped();
    if (ret != kIOReturnSuccess) { return ret; }

    switch (this->attributes.getFamily()) {
        case SMCRSChipFamily::SeaIslands:
        case SMCRSChipFamily::SouthernIslands:
        case SMCRSChipFamily::VolcanicIslands:
            return this->smu7GetTemp(data);
        case SMCRSChipFamily::ArcticIslands:
            if (this->attributes.isTHM11()) {
                return this->thm11GetTemp(data);
            } else {
                return this->thm9GetTemp(data);
            }
        case SMCRSChipFamily::Raven:
        case SMCRSChipFamily::Navi:
            return this->thm10GetTemp(data);
        default:
            return kIOReturnUnsupported;
    }
}

bool SMCRSCard::supportsPower() { return this->attributes.supportsPower(); }

IOReturn SMCRSCard::getPower(float *data) {
    auto ret = this->ensureRMMIOMapped();
    if (ret != kIOReturnSuccess) { return ret; }

    if (!this->supportsPower()) { return kIOReturnUnsupported; }
    switch (this->attributes.getFamily()) {
        case SMCRSChipFamily::SeaIslands:
            return this->smu7GetPowerPMStatus(data);
        case SMCRSChipFamily::SouthernIslands:
        case SMCRSChipFamily::VolcanicIslands:
            return this->smu7GetPowerSMC(data);
        case SMCRSChipFamily::ArcticIslands:
            return this->smu9GetPower(data);
        default:
            return kIOReturnUnsupported;
    }
}
