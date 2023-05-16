//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "KeyImplementations.hpp"
#include <os/log.h>

SMC_RESULT GPUTempProvider::readAccess() {
    UInt16 temp = provider->getTemperature(index);

    UInt16 *ptr = reinterpret_cast<UInt16 *>(data);
    *ptr = VirtualSMCAPI::encodeIntSp(SmcKeyTypeSp78, temp);

    return SmcSuccess;
}
