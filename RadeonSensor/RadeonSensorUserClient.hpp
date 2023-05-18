//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef RadeonSensorUserClient_hpp
#define RadeonSensorUserClient_hpp

#include "kern_rsensor.hpp"
#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>

struct RadeonSensorSelector {
    enum : uint32_t {
        GetVersion = 0,
        GetCardCount,
        GetTemperatures,
    };
};

class RadeonSensorUserClient : public IOUserClient {
    OSDeclareDefaultStructors(RadeonSensorUserClient);

    private:
    RadeonSensor *mProvider;
    task_t mTask;

    public:
    virtual bool initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties) override;

    // IOUserClient methods
    virtual void stop(IOService *provider) override;
    virtual bool start(IOService *provider) override;

    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
        IOExternalMethodDispatch *dispatch, OSObject *target, void *reference) override;
};

#endif /* RadeonSensorUserClient.hpp */
