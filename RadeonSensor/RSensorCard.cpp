//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RSensorCard.hpp"
#include "RadeonChipsets.hpp"
#include <Headers/kern_iokit.hpp>

OSDefineMetaClassAndStructors(RSensorCard, OSObject);

bool RSensorCard::initialise(IOPCIDevice *radeonDevice) {
    this->deviceId = WIOKit::readPCIConfigValue(radeonDevice, WIOKit::kIOPCIConfigDeviceID);

    if ((this->deviceId >= 0x7301 && this->deviceId <= 0x73FF)) {
        chipFamily = ChipFamily::Navi;
        DBGLOG("rsensor", "Family: Navi");
    } else if (this->deviceId == 0x15DD || this->deviceId == 0x15D8 || this->deviceId == 0x164C ||
               this->deviceId == 0x1636 || this->deviceId == 0x15E7 || this->deviceId == 0x1638) {
        chipFamily = ChipFamily::Raven;
        DBGLOG("rsensor", "Family: Raven");
    } else if ((this->deviceId >= 0x66A0 && this->deviceId <= 0x66AF) ||
               (this->deviceId >= 0x6860 && this->deviceId <= 0x687F)) {
        chipFamily = ChipFamily::ArcticIslands;
        DBGLOG("rsensor", "Family: Arctic Islands");
    } else if ((this->deviceId >= 0x67C0 && this->deviceId <= 0x67FF) ||
               (this->deviceId >= 0x6980 && this->deviceId <= 0x699F)) {
        chipFamily = ChipFamily::VolcanicIslands;
        DBGLOG("rsensor", "Family: Volcanic Islands");
    } else if ((this->deviceId >= 0x67A0 && this->deviceId <= 0x67BF) ||
               (this->deviceId >= 0x6900 && this->deviceId <= 0x693F) ||
               (this->deviceId >= 0x6600 && this->deviceId <= 0x663F) ||
               (this->deviceId >= 0x6640 && this->deviceId <= 0x666F)) {
        chipFamily = ChipFamily::SouthernIslands;
        DBGLOG("rsensor", "Family: Southern Islands");
    } else if ((this->deviceId >= 0x6780 && this->deviceId <= 0x679F) ||
               (this->deviceId >= 0x6800 && this->deviceId <= 0x683F)) {
        chipFamily = ChipFamily::SeaIslands;
        DBGLOG("rsensor", "Family: Sea Islands");
    } else {
        SYSLOG("rsensor", "Unsupported card 0x%04X", this->deviceId);
        return false;
    }

    if (chipFamily >= ChipFamily::SouthernIslands) {
        this->mmioMap = radeonDevice->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress5);
        if (!this->mmioMap || !this->mmioMap->getLength()) {
            DBGLOG("rsensor", "Failed to map BAR5");
            return false;
        }
        this->mmioBase = reinterpret_cast<volatile UInt8 *>(this->mmioMap->getVirtualAddress());
        DBGLOG("rsensor", "Using BAR5, located at %p", this->mmioBase);
    } else {
        this->mmioMap = radeonDevice->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress2);
        if (!this->mmioMap || !this->mmioMap->getLength()) {
            DBGLOG("rsensor", "Failed to map BAR2");
            return false;
        }
        this->mmioBase = reinterpret_cast<volatile UInt8 *>(this->mmioMap->getVirtualAddress());
        DBGLOG("rsensor", "Using BAR2, located at %p", this->mmioBase);
    }

    return true;
}

IOReturn RSensorCard::getTemperature(UInt16 *data) {
    switch (this->chipFamily) {
        case ChipFamily::SeaIslands:
            [[fallthrough]];
        case ChipFamily::SouthernIslands:
            return tahitiTemperature(data);
        case ChipFamily::VolcanicIslands:
            return arcticTemperature(data);
        case ChipFamily::ArcticIslands:
            [[fallthrough]];
        case ChipFamily::Raven:
            [[fallthrough]];
        case ChipFamily::Navi:
            return vegaTemperature(data);
        default:
            return kIOReturnError;
    }
}

UInt32 RSensorCard::read_smc(UInt32 reg) {
    UInt32 r;
    write32(SMC_IND_INDEX_0, (reg));
    r = read32(SMC_IND_DATA_0);
    return r;
}

UInt32 RSensorCard::read_ind(UInt32 reg) {
    // unsigned long flags;
    UInt32 r;
    // spin_lock_irqsave(&rdev->smc_idx_lock, flags);
    write32(mmSMC_IND_INDEX_11, reg);
    r = read32(mmSMC_IND_DATA_11);
    // spin_unlock_irqrestore(&rdev->smc_idx_lock, flags);
    return r;
}

UInt32 RSensorCard::read32(UInt32 reg) { return OSReadLittleInt32((mmioBase), reg); }

void RSensorCard::write32(UInt32 reg, UInt32 val) { return OSWriteLittleInt32((mmioBase), reg, val); }

IOReturn RSensorCard::tahitiTemperature(UInt16 *data) {
    UInt32 temp, actual_temp = 0;
    for (int i = 0; i < 1000; i++) {    // attempts to ready
        temp = (read32(CG_SI_THERMAL_STATUS) & CTF_TEMP_MASK) >> CTF_TEMP_SHIFT;
        if ((temp >> 10) & 1) {
            actual_temp = 0;
        } else if ((temp >> 9) & 1) {
            actual_temp = 255;
        } else {
            actual_temp = temp;    //(temp >> 1) & 0xff;
            break;
        }
        IOSleep(10);
    }

    *data = (UInt16)(actual_temp & 0x1ff);
    // data[1] = 0;
    return kIOReturnSuccess;
}

IOReturn RSensorCard::arcticTemperature(UInt16 *data) {
    UInt32 temp, actual_temp = 0;
    for (int i = 0; i < 1000; i++) {    // attempts to ready
        temp = (read_ind(CG_CI_MULT_THERMAL_STATUS) & CI_CTF_TEMP_MASK) >> CI_CTF_TEMP_SHIFT;
        if ((temp >> 10) & 1) {
            actual_temp = 0;
        } else if ((temp >> 9) & 1) {
            actual_temp = 255;
        } else {
            actual_temp = temp & 0x1ff;    //(temp >> 1) & 0xff;
            break;
        }
        IOSleep(10);
    }

    *data = (UInt16)(actual_temp & 0x1ff);
    // data[1] = 0;
    return kIOReturnSuccess;
}

IOReturn RSensorCard::vegaTemperature(UInt16 *data) {
    UInt32 temp, actual_temp = 0;

    temp = read32(mmTHM_TCON_CUR_TMP) >> THM_TCON_CUR_TMP__CUR_TEMP__SHIFT;
    actual_temp = temp & 0x1ff;
    *data = (UInt16)actual_temp;

    return kIOReturnSuccess;
}
