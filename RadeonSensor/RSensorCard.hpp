//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef RadeonCard_hpp
#define RadeonCard_hpp
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

    UInt32 deviceId {0};
    ChipFamily chipFamily {ChipFamily::Unknown};

    volatile UInt32 *rmmioPtr {nullptr};
    IOMemoryMap *rmmio {nullptr};
    int cardNumber {0};

    UInt32 readIndirect(UInt32 reg);
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

#endif /* RadeonCard.hpp */
