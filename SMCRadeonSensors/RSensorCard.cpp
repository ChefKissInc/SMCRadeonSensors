//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RSensorCard.hpp"
#include "RadeonChipsets.hpp"
#include <Headers/kern_iokit.hpp>

OSDefineMetaClassAndStructors(RSensorCard, OSObject);

bool RSensorCard::initialise(IOPCIDevice *device) {
    if (!device) { return false; }
    this->dev = device;

    auto deviceID = WIOKit::readPCIConfigValue(this->dev, WIOKit::kIOPCIConfigDeviceID);
    switch (deviceID) {
        case 0x15DD:
            [[fallthrough]];
        case 0x15D8:
            [[fallthrough]];
        case 0x164C:
            [[fallthrough]];
        case 0x1636:
            [[fallthrough]];
        case 0x15E7:
            [[fallthrough]];
        case 0x1638:
            this->chipFamily = ChipFamily::Raven;
            DBGLOG("RSCard", "Raven");
            break;
        case 0x66A0 ... 0x66AF:
            this->thm11 = true;
            [[fallthrough]];
        case 0x6860 ... 0x687F:
            [[fallthrough]];
        case 0x69A0 ... 0x69AF:
            this->chipFamily = ChipFamily::ArcticIslands;
            DBGLOG("RSCard", "Arctic Islands");
            break;
        case 0x67C0 ... 0x67FF:
            [[fallthrough]];
        case 0x6980 ... 0x699F:
            this->chipFamily = ChipFamily::VolcanicIslands;
            DBGLOG("RSCard", "Volcanic Islands");
            break;
        case 0x6600 ... 0x666F:
            [[fallthrough]];
        case 0x67A0 ... 0x67BF:
            [[fallthrough]];
        case 0x6900 ... 0x693F:
            this->chipFamily = ChipFamily::SouthernIslands;
            DBGLOG("RSCard", "Southern Islands");
            break;
        case 0x6780 ... 0x679F:
            [[fallthrough]];
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

    auto bar5 = this->chipFamily >= ChipFamily::SouthernIslands;
    this->rmmio = this->dev->mapDeviceMemoryWithRegister(bar5 ? kIOPCIConfigBaseAddress5 : kIOPCIConfigBaseAddress2);
    if (!this->rmmio || !this->rmmio->getLength()) {
        SYSLOG("RSCard", "Failed to map BAR%d", bar5 ? 5 : 2);
        OSSafeReleaseNULL(this->rmmio);
        return false;
    }
    this->rmmioPtr = reinterpret_cast<volatile UInt32 *>(this->rmmio->getVirtualAddress());
    if (!this->rmmioPtr) {
        SYSLOG("RSCard", "Failed to get virtual address for BAR%d", bar5 ? 5 : 2);
        OSSafeReleaseNULL(this->rmmio);
        return false;
    }
    DBGLOG("RSCard", "Using BAR%d, located at %p", bar5 ? 5 : 2, this->rmmioPtr);

    return true;
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
    bool soc15 = this->chipFamily >= ChipFamily::ArcticIslands;
    if ((reg * 4) < this->rmmio->getLength()) {
        return this->rmmioPtr[reg];
    } else {
        this->rmmioPtr[soc15 ? mmPCIE_INDEX2 : mmPCIE_INDEX] = reg;
        return this->rmmioPtr[soc15 ? mmPCIE_DATA2 : mmPCIE_DATA];
    }
}

void RSensorCard::writeReg32(UInt32 reg, UInt32 val) {
    bool soc15 = this->chipFamily >= ChipFamily::ArcticIslands;
    if ((reg * 4) < this->rmmio->getLength()) {
        this->rmmioPtr[reg] = val;
    } else {
        this->rmmioPtr[soc15 ? mmPCIE_INDEX2 : mmPCIE_INDEX] = reg;
        this->rmmioPtr[soc15 ? mmPCIE_DATA2 : mmPCIE_DATA] = val;
    }
}

IOReturn RSensorCard::getTempSI(UInt16 *data) {
    auto regValue = this->readIndirectSMCSI(ixCG_MULT_THERMAL_STATUS);
    *data = (regValue & 0x200) ? 255 : (regValue & 0x1FF);
    return kIOReturnSuccess;
}

IOReturn RSensorCard::getTempVI(UInt16 *data) {
    auto regValue = this->readIndirectSMCVI(ixCG_MULT_THERMAL_STATUS);
    *data = (regValue & 0x200) ? 255 : (regValue & 0x1FF);
    return kIOReturnSuccess;
}

IOReturn RSensorCard::getTempAI(UInt16 *data) {
    auto regValue =
        this->readReg32(THM_BASE + (this->thm11 ? mmCG_MULT_THERMAL_STATUS_THM11 : mmCG_MULT_THERMAL_STATUS_THM9));
    *data = ((regValue & CTF_TEMP_MASK) >> CTF_TEMP_SHIFT) & 0x1FF;
    return kIOReturnSuccess;
}

IOReturn RSensorCard::getTempRV(UInt16 *data) {
    auto regValue = this->readReg32(THM_BASE + mmTHM_TCON_CUR_TMP);
    *data = ((regValue & CUR_TEMP_MASK) >> CUR_TEMP_SHIFT) / 8;
    if (regValue & CUR_TEMP_RANGE_SEL_MASK) { *data -= 49; }
    return kIOReturnSuccess;
}
