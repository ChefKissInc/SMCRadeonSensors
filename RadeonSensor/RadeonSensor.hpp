//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef RadeonSensor_hpp
#define RadeonSensor_hpp

#include "RadeonCard.hpp"

class RadeonSensor : public IOService {
    OSDeclareDefaultStructors(RadeonSensor);

    public:
    virtual bool init(OSDictionary *dictionary) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual void free(void) override;

    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;

    UInt16 getTemperature(UInt16 card);
    UInt16 getCardCount();

    private:
    UInt16 cardCount = 0;
    RadeonCard **radeonCards = nullptr;
};

#endif /* RadeonSensor.hpp */
