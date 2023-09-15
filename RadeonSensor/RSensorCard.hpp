//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#pragma once
#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>

enum struct ChipFamily {
    Unknown,
    SeaIslands,
    SouthernIslands,
    VolcanicIslands,
    ArcticIslands,
    Raven,
    Navi,
};

class RSensorCard : public OSObject {
    OSDeclareDefaultStructors(RSensorCard);

    ChipFamily chipFamily {ChipFamily::Unknown};

    IOPCIDevice *dev {nullptr};
    volatile UInt32 *rmmioPtr {nullptr};
    IOMemoryMap *rmmio {nullptr};
    bool thm11 = false;

    IOReturn setRMMIOIfNecessary();

    UInt32 readIndirectSMCSI(UInt32 reg);
    UInt32 readIndirectSMCVI(UInt32 reg);
    UInt32 readReg32(UInt32 reg);
    void writeReg32(UInt32 reg, UInt32 val);

    IOReturn getTempSI(UInt16 *data);
    IOReturn getTempVI(UInt16 *data);
    IOReturn getTempAI(UInt16 *data);
    IOReturn getTempRV(UInt16 *data);

    public:
    bool initialise(IOPCIDevice *radeonDevice);
    IOReturn getTemperature(UInt16 *data);
};
