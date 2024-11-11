// Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5.
// See LICENSE for details.

#include "KeyImplementations.hpp"

SMC_RESULT RGPUTempValue::readAccess() {
    auto value = this->provider->getTemperature(this->index);
    *reinterpret_cast<UInt16 *>(this->data) = VirtualSMCAPI::encodeIntSp(this->type, value);
    return SmcSuccess;
}

SMC_RESULT RGPUPowerValue::readAccess() {
    auto value = this->provider->getPower(this->index);
    *reinterpret_cast<UInt16 *>(data) = VirtualSMCAPI::encodeSp(this->type, static_cast<double>(value));
    return SmcSuccess;
}
