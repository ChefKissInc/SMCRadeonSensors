//  Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5. See LICENSE for
//  details.

#pragma once
#include "SMCRadeonSensors.hpp"
#include <VirtualSMCSDK/AppleSmc.h>

class RadeonSMCValue : public VirtualSMCValue {
    protected:
    UInt32 index {0};
    PRODUCT_NAME *provider {nullptr};

    public:
    RadeonSMCValue(PRODUCT_NAME *provider, UInt32 index) : index {index}, provider {provider} {}
};

class RGPUTempValue : public RadeonSMCValue {
    using RadeonSMCValue::RadeonSMCValue;

    protected:
    SMC_RESULT readAccess() APPLE_KEXT_OVERRIDE;
};

class RGPUPowerValue : public RadeonSMCValue {
    using RadeonSMCValue::RadeonSMCValue;

    protected:
    SMC_RESULT readAccess() APPLE_KEXT_OVERRIDE;
};
