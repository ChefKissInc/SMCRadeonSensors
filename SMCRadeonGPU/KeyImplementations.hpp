//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef KeyImplementations_hpp
#define KeyImplementations_hpp
#include "SMCRadeonGPU.hpp"
#include <VirtualSMCSDK/AppleSmc.h>

class RadeonSMCValue : public VirtualSMCValue {
    protected:
    size_t index;
    RadeonSensor *provider;

    public:
    RadeonSMCValue(RadeonSensor *provider, size_t index) : index {index}, provider {provider} {}
};

class RGPUTempValue : public RadeonSMCValue {
    using RadeonSMCValue::RadeonSMCValue;

    protected:
    SMC_RESULT readAccess() override;
};

#endif /* KeyImplementations_hpp */
