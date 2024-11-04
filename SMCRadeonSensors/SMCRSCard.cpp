//  Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5. See LICENSE for
//  details.

#include "SMCRSCard.hpp"
#include "AMDCommon.hpp"
#include <Headers/kern_iokit.hpp>

OSDefineMetaClassAndStructors(SMCRSCard, OSObject);

IOReturn SMCRSCard::ensureRMMIOMapped() {
    if (this->rmmio == nullptr) {
        this->dev->setMemoryEnable(true);
        this->dev->setBusMasterEnable(true);
        auto bar5 = this->chipFamily >= ChipFamily::SouthernIslands;
        this->rmmio =
            this->dev->mapDeviceMemoryWithRegister(bar5 ? kIOPCIConfigBaseAddress5 : kIOPCIConfigBaseAddress2);
        if (this->rmmio == nullptr || !this->rmmio->getLength()) {
            SYSLOG("RSCard", "Failed to map BAR%d", bar5 ? 5 : 2);
            OSSafeReleaseNULL(this->rmmio);
            return kIOReturnDeviceError;
        }
        this->rmmioPtr = reinterpret_cast<volatile UInt32 *>(this->rmmio->getVirtualAddress());
        if (this->rmmioPtr == nullptr) {
            SYSLOG("RSCard", "Failed to get virtual address for BAR%d", bar5 ? 5 : 2);
            OSSafeReleaseNULL(this->rmmio);
            return kIOReturnDeviceError;
        }
        DBGLOG("RSCard", "Using BAR%d, located at %p", bar5 ? 5 : 2, this->rmmioPtr);
    }

    return kIOReturnSuccess;
}

UInt32 SMCRSCard::readReg32(UInt32 reg) {
    if (this->ensureRMMIOMapped() != kIOReturnSuccess) { return 0xFFFFFFFF; }

    bool soc15 = this->chipFamily >= ChipFamily::ArcticIslands;
    if ((reg * 4) < this->rmmio->getLength()) {
        return this->rmmioPtr[reg];
    } else {
        this->rmmioPtr[soc15 ? mmPCIE_INDEX2 : mmPCIE_INDEX] = reg;
        return this->rmmioPtr[soc15 ? mmPCIE_DATA2 : mmPCIE_DATA];
    }
}

void SMCRSCard::writeReg32(UInt32 reg, UInt32 val) {
    if (this->ensureRMMIOMapped() != kIOReturnSuccess) { return; }

    bool soc15 = this->chipFamily >= ChipFamily::ArcticIslands;
    if ((reg * 4) < this->rmmio->getLength()) {
        this->rmmioPtr[reg] = val;
    } else {
        this->rmmioPtr[soc15 ? mmPCIE_INDEX2 : mmPCIE_INDEX] = reg;
        this->rmmioPtr[soc15 ? mmPCIE_DATA2 : mmPCIE_DATA] = val;
    }
}

UInt32 SMCRSCard::readIndirectSMCSI(UInt32 reg) {
    this->writeReg32(mmSMC_IND_INDEX_0, reg);
    return this->readReg32(mmSMC_IND_DATA_0);
}

UInt32 SMCRSCard::readIndirectSMCVI(UInt32 reg) {
    this->writeReg32(mmSMC_IND_INDEX_11, reg);
    return this->readReg32(mmSMC_IND_DATA_11);
}

IOReturn SMCRSCard::getTempSI(UInt16 *data) {
    auto ctfTemp = GET_THERMAL_STATUS_CTF_TEMP(this->readIndirectSMCSI(ixCG_MULT_THERMAL_STATUS));
    *data = (ctfTemp & 0x200) ? 255 : (ctfTemp & 0x1FF);
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::getTempVI(UInt16 *data) {
    auto ctfTemp = GET_THERMAL_STATUS_CTF_TEMP(this->readIndirectSMCVI(ixCG_MULT_THERMAL_STATUS));
    *data = (ctfTemp & 0x200) ? 255 : (ctfTemp & 0x1FF);
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::getTempTHM9(UInt16 *data) {
    auto reg = this->readReg32(THM_BASE + mmCG_MULT_THERMAL_STATUS_THM9);
    *data = GET_THERMAL_STATUS_CTF_TEMP(reg) & 0x1FF;
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::getTempTHM11(UInt16 *data) {
    auto reg = this->readReg32(THM_BASE + mmCG_MULT_THERMAL_STATUS_THM11);
    *data = GET_THERMAL_STATUS_CTF_TEMP(reg) & 0x1FF;
    return kIOReturnSuccess;
}

IOReturn SMCRSCard::getTempRV(UInt16 *data) {
    auto reg = this->readReg32(THM_BASE + mmTHM_TCON_CUR_TMP);
    *data = GET_TCON_CUR_TEMP(reg) / 8;
    if (reg & CUR_TEMP_RANGE_SEL) { *data -= 49; }
    return kIOReturnSuccess;
}

bool SMCRSCard::initialise(IOPCIDevice *device) {
    if (device == nullptr) { return false; }

    this->dev = device;

    auto deviceID = WIOKit::readPCIConfigValue(this->dev, WIOKit::kIOPCIConfigDeviceID);
    switch (deviceID) {
        case 0x15DD:
        case 0x15D8:
        case 0x164C:
        case 0x1636:
        case 0x15E7:
        case 0x1638:
            this->chipFamily = ChipFamily::Raven;
            DBGLOG("RSCard", "Raven");
            break;
        case 0x66A0 ... 0x66AF:
            this->chipFamily = ChipFamily::ArcticIslandsTHM11;
            DBGLOG("RSCard", "Arctic Islands (THM 11)");
            break;
        case 0x6860 ... 0x687F:
        case 0x69A0 ... 0x69AF:
            this->chipFamily = ChipFamily::ArcticIslands;
            DBGLOG("RSCard", "Arctic Islands");
            break;
        case 0x67C0 ... 0x67FF:
        case 0x6980 ... 0x699F:
            this->chipFamily = ChipFamily::VolcanicIslands;
            DBGLOG("RSCard", "Volcanic Islands");
            break;
        case 0x6600 ... 0x666F:
        case 0x67A0 ... 0x67BF:
        case 0x6900 ... 0x693F:
            this->chipFamily = ChipFamily::SouthernIslands;
            DBGLOG("RSCard", "Southern Islands");
            break;
        case 0x6780 ... 0x679F:
        case 0x6800 ... 0x683F:
            this->chipFamily = ChipFamily::SeaIslands;
            DBGLOG("RSCard", "Sea Islands");
            break;
        case 0x7301 ... 0x73FF:
            this->chipFamily = ChipFamily::Navi;
            DBGLOG("RSCard", "Navi");
            break;
        default:
            SYSLOG("RSCard", "Unsupported card 0x%04X", deviceID);
            return false;
    }

    return true;
}

IOReturn SMCRSCard::getTemperature(UInt16 *data) {
    switch (this->chipFamily) {
        case ChipFamily::SeaIslands:
        case ChipFamily::SouthernIslands:
            return this->getTempSI(data);
        case ChipFamily::VolcanicIslands:
            return this->getTempVI(data);
        case ChipFamily::ArcticIslands:
            return this->getTempTHM9(data);
        case ChipFamily::ArcticIslandsTHM11:
            return this->getTempTHM11(data);
        case ChipFamily::Raven:
        case ChipFamily::Navi:
            return this->getTempRV(data);
        default:
            return kIOReturnError;
    }
}
