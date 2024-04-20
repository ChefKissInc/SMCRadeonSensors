//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "KeyImplementations.hpp"

SMC_RESULT RGPUTempValue::readAccess() {
    auto value = this->provider->getTemperature(this->index);
    *reinterpret_cast<UInt16 *>(this->data) = VirtualSMCAPI::encodeIntSp(SmcKeyTypeSp78, value);
    return SmcSuccess;
}
