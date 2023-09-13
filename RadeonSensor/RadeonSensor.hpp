//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#pragma once
#include <Headers/kern_iokit.hpp>
#include <IOKit/IOService.h>

class EXPORT RadeonSensor : public IOService {
    OSDeclareDefaultStructors(RadeonSensor);

    OSArray *cards {nullptr};

    public:
    IOService *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void free() APPLE_KEXT_OVERRIDE;

    virtual UInt16 getTemperature(UInt16 card);
    virtual UInt16 getCardCount();
};
