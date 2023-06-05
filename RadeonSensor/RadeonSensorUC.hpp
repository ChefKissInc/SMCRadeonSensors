//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#pragma once
#include "RSensor.hpp"
#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>

struct RadeonSensorSelector {
    enum : uint32_t {
        GetVersionLength = 0,
        GetVersion,
        GetCardCount,
        GetTemperatures,
    };
};

class RadeonSensorUserClient : public IOUserClient {
    OSDeclareDefaultStructors(RadeonSensorUserClient);

    private:
    RadeonSensor *rsensor;
    task_t owningTask;

    public:
    bool initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties) override;
    void stop(IOService *provider) override;
    bool start(IOService *provider) override;
    IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *dispatch,
        OSObject *target, void *reference) override;
};
