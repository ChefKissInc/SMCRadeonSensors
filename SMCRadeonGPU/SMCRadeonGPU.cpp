//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "SMCRadeonGPU.hpp"
#include "KeyImplementations.hpp"
#include <Headers/kern_util.hpp>
#include <Headers/kern_version.hpp>

OSDefineMetaClassAndStructors(SMCRadeonGPU, IOService);

bool ADDPR(debugEnabled) = true;
uint32_t ADDPR(debugPrintDelay) = 0;

IOService *SMCRadeonGPU::probe(IOService *provider, SInt32 *score) {
    if (!IOService::probe(provider, score)) {
        SYSLOG("init", "super::probe failed");
        return nullptr;
    }

    this->rsensor = OSDynamicCast(RadeonSensor, provider);
    if (!this->rsensor) {
        SYSLOG("init", "Null rsensor");
        return nullptr;
    }

    auto gpuCount = this->rsensor->getCardCount();
    for (auto i = 0; i < gpuCount; i++) {
        VirtualSMCAPI::addKey(KeyTGxD(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->rsensor, i)));
        VirtualSMCAPI::addKey(KeyTGxP(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->rsensor, i)));
        VirtualSMCAPI::addKey(KeyTGxd(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->rsensor, i)));
        VirtualSMCAPI::addKey(KeyTGxp(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->rsensor, i)));
    }

    qsort(const_cast<VirtualSMCKeyValue *>(vsmcPlugin.data.data()), vsmcPlugin.data.size(), sizeof(VirtualSMCKeyValue),
        VirtualSMCKeyValue::compare);

    return this;
}

bool SMCRadeonGPU::start(IOService *provider) {
    if (!IOService::start(provider)) {
        SYSLOG("init", "super::start failed");
        return false;
    }

    this->setProperty("VersionInfo", kextVersion);

    this->vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    if (!this->vsmcNotifier) {
        SYSLOG("init", "VirtualSMCAPI::registerHandler failed");
        return false;
    }

    return true;
}

bool SMCRadeonGPU::vsmcNotificationHandler(void *target, void *, IOService *newService, IONotifier *) {
    if (!target || !newService) {
        SYSLOG("smcrgpu", "Null notification");
        return false;
    }

    auto &plugin = static_cast<SMCRadeonGPU *>(target)->vsmcPlugin;
    auto ret = newService->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, target, &plugin, nullptr, nullptr);
    if (ret == kIOReturnSuccess) {
        DBGLOG("smcrgpu", "Submitted plugin");
        return true;
    } else if (ret != kIOReturnUnsupported) {
        DBGLOG("smcrgpu", "Plugin submission failure %X", ret);
        return false;
    } else {
        DBGLOG("smcrgpu", "Plugin submitted to non-VSMC");
        return false;
    }
}

void SMCRadeonGPU::stop(IOService *) { PANIC("smcrgpu", "Called stop!!!"); }

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
    // Report success but actually do not start and let I/O Kit unload us.
    // This works better and increases boot speed in some cases.
    lilu_get_boot_args("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) =
        checkKernelArgument("-vsmcdbg") || checkKernelArgument("-rsensordbg") || checkKernelArgument("-liludbgall");
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) {
    // It is not safe to unload VirtualSMC plugins!
    return KERN_FAILURE;
}
