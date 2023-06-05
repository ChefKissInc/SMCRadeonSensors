//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#pragma once
#include <Headers/kern_iokit.hpp>
#include <IOKit/IOService.h>

class EXPORT RadeonSensor : public IOService {
    OSDeclareDefaultStructors(RadeonSensor);

    public:
    IOService *probe(IOService *provider, SInt32 *score) override;
    bool start(IOService *provider) override;
    void free() override;

    virtual UInt16 getTemperature(UInt16 card);
    virtual UInt16 getCardCount();
};

struct RSensor {
    OSArray *cards {nullptr};

    void init();
    void deinit();
    void populateCards();
};
