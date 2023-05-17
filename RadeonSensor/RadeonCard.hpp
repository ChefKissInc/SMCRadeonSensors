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
};

class RadeonCard : public OSObject {
    OSDeclareDefaultStructors(RadeonCard);

    public:
    bool initialise(IOPCIDevice *radeonDevice, UInt32 chipID);
    IOReturn getTemperature(UInt16 *data);

    private:
    UInt32 deviceId {0};
    ChipFamily chipFamily {ChipFamily::Unknown};

    volatile UInt8 *mmioBase {nullptr};
    IOMemoryMap *mmioMap {nullptr};
    UInt32 caps {0};
    UInt32 tReg {0};
    int cardNumber {0};

    UInt32 read32(UInt32 reg);
    void write32(UInt32 reg, UInt32 val);
    UInt32 read_ind(UInt32 reg);
    UInt32 read_smc(UInt32 reg);

    IOReturn tahitiTemperature(UInt16 *data);
    IOReturn arcticTemperature(UInt16 *data);
    IOReturn vegaTemperature(UInt16 *data);
};

#endif /* RadeonCard.hpp */
