//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RadeonSensorUserClient.hpp"
#include <Headers/kern_util.hpp>
#include <IOKit/IOLib.h>

#define super IOUserClient

OSDefineMetaClassAndStructors(RadeonSensorUserClient, IOUserClient);

bool RadeonSensorUserClient::initWithTask(task_t owningTask, void *securityToken, UInt32 type,
    OSDictionary *properties) {
    if (!owningTask) return false;

    if (!super::initWithTask(owningTask, securityToken, type)) {
        IOLog("RadeonSensorUserClient::initWithTask: super::initWithTask failed\n");
        return false;
    }

    IOLog("RadeonSensorUserClient::initWithTask\n");

    mTask = owningTask;
    mProvider = NULL;

    return true;
}

bool RadeonSensorUserClient::start(IOService *provider) {
    if (!super::start(provider)) {
        IOLog("RadeonSensorUserClient::start: super::start failed\n");
        return false;
    }

    this->mProvider = OSDynamicCast(RadeonSensor, provider);
    if (!this->mProvider) {
        IOLog("RadeonSensorUserClient::initWithTask: null mProvider\n");
        return false;
    }

    IOLog("RadeonSensorUserClient::start\n");
    return true;
}

void RadeonSensorUserClient::stop(IOService *provider) {
    IOLog("RadeonSensorUserClient::stop\n");
    this->mProvider = nullptr;
    super::stop(provider);
}

IOReturn RadeonSensorUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
    IOExternalMethodDispatch *dispatch, OSObject *target, void *reference) {
    switch (selector) {
        case RadeonSensorSelector::GetVersion: {
            static char version[] = xStringify(MODULE_VERSION);
            if (arguments->structureOutput) {
                arguments->scalarOutputCount = 0;
                arguments->structureOutputSize = sizeof(version);
                memcpy(arguments->structureOutput, version, sizeof(version));
            } else {
                arguments->scalarOutputCount = 1;
                arguments->scalarOutput[0] = sizeof(version);
                arguments->structureOutputSize = 0;
            }
            break;
        }
        case RadeonSensorSelector::GetCardCount: {
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = this->mProvider->getCardCount();
            arguments->structureOutputSize = 0;
            break;
        }
        case RadeonSensorSelector::GetTemperatures: {
            UInt16 cardCount = this->mProvider->getCardCount();
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = cardCount;

            arguments->structureOutputSize = cardCount * sizeof(UInt16);
            for (size_t i = 0; i < cardCount; i++) {
                static_cast<UInt16 *>(arguments->structureOutput)[i] = this->mProvider->getTemperature(i);
            }
            break;
        }
    }

    return kIOReturnSuccess;
}
