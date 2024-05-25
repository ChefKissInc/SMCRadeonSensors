//  Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5. See LICENSE for
//  details.

#include "KeyImplementations.hpp"

SMC_RESULT RGPUTempValue::readAccess() {
    auto value = this->provider->getTemperature(this->index);
    *reinterpret_cast<UInt16 *>(this->data) = VirtualSMCAPI::encodeIntSp(SmcKeyTypeSp78, value);
    return SmcSuccess;
}
