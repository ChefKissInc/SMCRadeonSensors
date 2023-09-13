//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RSensorCard.hpp"
#include "RadeonChipsets.hpp"
#include <Headers/kern_iokit.hpp>

OSDefineMetaClassAndStructors(RSensorCard, OSObject);

bool RSensorCard::initialise(IOPCIDevice *device) {
    if (!device) { return false; }
    this->dev = device;

    WIOKit::awaitPublishing(this->dev);
    this->dev->setMemoryEnable(true);

    UInt32 deviceId = WIOKit::readPCIConfigValue(device, WIOKit::kIOPCIConfigDeviceID);
    if (deviceId >= 0x7301 && deviceId <= 0x73FF) {
        this->chipFamily = ChipFamily::Navi;
        DBGLOG("rsensor", "Family: Navi");
    } else if (deviceId == 0x15DD || deviceId == 0x15D8 || deviceId == 0x164C || deviceId == 0x1636 ||
               deviceId == 0x15E7 || deviceId == 0x1638) {
        this->chipFamily = ChipFamily::Raven;
        DBGLOG("rsensor", "Family: Raven");
    } else if ((deviceId >= 0x66A0 && deviceId <= 0x66AF) || (deviceId >= 0x6860 && deviceId <= 0x687F)) {
        this->chipFamily = ChipFamily::ArcticIslands;
        DBGLOG("rsensor", "Family: Arctic Islands");
    } else if ((deviceId >= 0x67C0 && deviceId <= 0x67FF) || (deviceId >= 0x6980 && deviceId <= 0x699F)) {
        this->chipFamily = ChipFamily::VolcanicIslands;
        DBGLOG("rsensor", "Family: Volcanic Islands");
    } else if ((deviceId >= 0x67A0 && deviceId <= 0x67BF) || (deviceId >= 0x6900 && deviceId <= 0x693F) ||
               (deviceId >= 0x6600 && deviceId <= 0x663F) || (deviceId >= 0x6640 && deviceId <= 0x666F)) {
        this->chipFamily = ChipFamily::SouthernIslands;
        DBGLOG("rsensor", "Family: Southern Islands");
    } else if ((deviceId >= 0x6780 && deviceId <= 0x679F) || (deviceId >= 0x6800 && deviceId <= 0x683F)) {
        this->chipFamily = ChipFamily::SeaIslands;
        DBGLOG("rsensor", "Family: Sea Islands");
    } else {
        SYSLOG("rsensor", "Unsupported card 0x%04X", deviceId);
        return false;
    }

    return true;
}

IOReturn RSensorCard::setRMMIOIfNecessary() {
    if (LIKELY(this->rmmio && this->rmmio->getLength())) { return kIOReturnSuccess; }

    OSSafeReleaseNULL(this->rmmio);
    this->rmmioPtr = nullptr;

    auto bar5 = this->chipFamily >= ChipFamily::SouthernIslands;
    this->rmmio = this->dev->mapDeviceMemoryWithRegister(bar5 ? kIOPCIConfigBaseAddress5 : kIOPCIConfigBaseAddress2);
    if (!this->rmmio || !this->rmmio->getLength()) {
        SYSLOG("rsensor", "Failed to map BAR%d", bar5 ? 5 : 2);
        OSSafeReleaseNULL(this->rmmio);
        return kIOReturnNoMemory;
    }
    this->rmmioPtr = reinterpret_cast<volatile UInt32 *>(this->rmmio->getVirtualAddress());
    if (!this->rmmioPtr) {
        SYSLOG("rsensor", "Failed to get virtual address for BAR%d", bar5 ? 5 : 2);
        OSSafeReleaseNULL(this->rmmio);
        return kIOReturnNoMemory;
    }
    DBGLOG("rsensor", "Using BAR%, located at %p", bar5 ? 5 : 2, this->rmmioPtr);

    return kIOReturnSuccess;
}

IOReturn RSensorCard::getTemperature(UInt16 *data) {
    switch (this->chipFamily) {
        case ChipFamily::SeaIslands:
            [[fallthrough]];
        case ChipFamily::SouthernIslands:
            return this->getTempSI(data);
        case ChipFamily::VolcanicIslands:
            return this->getTempVI(data);
        case ChipFamily::ArcticIslands:
            return this->getTempAI(data);
        case ChipFamily::Raven:
            [[fallthrough]];
        case ChipFamily::Navi:
            return this->getTempRV(data);
        default:
            return kIOReturnError;
    }
}

UInt32 RSensorCard::readIndirectSMCSI(UInt32 reg) {
    this->writeReg32(mmSMC_IND_INDEX_0, reg);
    return this->readReg32(mmSMC_IND_DATA_0);
}

UInt32 RSensorCard::readIndirectSMCVI(UInt32 reg) {
    this->writeReg32(mmSMC_IND_INDEX_11, reg);
    return this->readReg32(mmSMC_IND_DATA_11);
}

UInt32 RSensorCard::readReg32(UInt32 reg) {
    if (this->setRMMIOIfNecessary() != kIOReturnSuccess) { return 0xFFFFFFFF; }

    auto soc15plus = this->chipFamily >= ChipFamily::ArcticIslands;
    if ((reg * 4) < this->rmmio->getLength()) {
        return this->rmmioPtr[reg];
    } else {
        this->rmmioPtr[soc15plus ? mmPCIE_INDEX2 : mmPCIE_INDEX] = reg;
        return this->rmmioPtr[soc15plus ? mmPCIE_DATA2 : mmPCIE_DATA];
    }
}

void RSensorCard::writeReg32(UInt32 reg, UInt32 val) {
    if (this->setRMMIOIfNecessary() != kIOReturnSuccess) { return; }

    auto soc15plus = this->chipFamily >= ChipFamily::ArcticIslands;
    if ((reg * 4) < this->rmmio->getLength()) {
        this->rmmioPtr[reg] = val;
    } else {
        this->rmmioPtr[soc15plus ? mmPCIE_INDEX2 : mmPCIE_INDEX] = reg;
        this->rmmioPtr[soc15plus ? mmPCIE_DATA2 : mmPCIE_DATA] = val;
    }
}

IOReturn RSensorCard::getTempSI(UInt16 *data) {
    UInt32 regValue = readIndirectSMCSI(ixCG_MULT_THERMAL_STATUS);
    *data = (regValue & 0x200) ? 255 : (regValue & 0x1FF);
    return kIOReturnSuccess;
}

IOReturn RSensorCard::getTempVI(UInt16 *data) {
    UInt32 regValue = readIndirectSMCVI(ixCG_MULT_THERMAL_STATUS);
    *data = (regValue & 0x200) ? 255 : (regValue & 0x1FF);
    return kIOReturnSuccess;
}

IOReturn RSensorCard::getTempAI(UInt16 *data) {
    *data = ((readReg32(THM_BASE + mmCG_MULT_THERMAL_STATUS) & CTF_TEMP_MASK) >> CTF_TEMP_SHIFT) & 0x1FF;
    return kIOReturnSuccess;
}

IOReturn RSensorCard::getTempRV(UInt16 *data) {
    auto regValue = readReg32(THM_BASE + mmTHM_TCON_CUR_TMP);
    *data = ((regValue & CUR_TEMP_MASK) >> CUR_TEMP_SHIFT) / 8;
    if (regValue & CUR_TEMP_RANGE_SEL_MASK) { *data -= 49; }
    return kIOReturnSuccess;
}
