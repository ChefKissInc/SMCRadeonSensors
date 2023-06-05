//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RadeonSensorUC.hpp"
#include <Headers/kern_util.hpp>
#include <IOKit/IOLib.h>

#define super IOUserClient

OSDefineMetaClassAndStructors(RadeonSensorUserClient, IOUserClient);

bool RadeonSensorUserClient::initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *) {
    if (!owningTask) return false;

    if (!super::initWithTask(owningTask, securityToken, type)) {
        SYSLOG("rsensoruc", "super::initWithTask failed\n");
        return false;
    }

    this->owningTask = owningTask;
    this->rsensor = nullptr;

    return true;
}

bool RadeonSensorUserClient::start(IOService *provider) {
    if (!super::start(provider)) {
        SYSLOG("rsensoruc", "super::start failed\n");
        return false;
    }

    this->rsensor = OSDynamicCast(RadeonSensor, provider);
    if (!this->rsensor) {
        SYSLOG("rsensoruc", "null mProvider\n");
        return false;
    }

    return true;
}

void RadeonSensorUserClient::stop(IOService *provider) {
    this->rsensor = nullptr;
    super::stop(provider);
}

IOReturn RadeonSensorUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
    IOExternalMethodDispatch *, OSObject *, void *) {
    static char version[] = xStringify(MODULE_VERSION);
    switch (selector) {
        case RadeonSensorSelector::GetVersionLength: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = sizeof(version);
            arguments->structureOutputSize = 0;
            break;
        }
        case RadeonSensorSelector::GetVersion: {
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = sizeof(version);
            memcpy(arguments->structureOutput, version, sizeof(version));
            break;
        }
        case RadeonSensorSelector::GetCardCount: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = this->rsensor->getCardCount();
            arguments->structureOutputSize = 0;
            break;
        }
        case RadeonSensorSelector::GetTemperatures: {
            UInt16 cardCount = this->rsensor->getCardCount();
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = cardCount * sizeof(UInt16);
            for (size_t i = 0; i < cardCount; i++) {
                static_cast<UInt16 *>(arguments->structureOutput)[i] = this->rsensor->getTemperature(i);
            }
            break;
        }
    }

    return kIOReturnSuccess;
}
