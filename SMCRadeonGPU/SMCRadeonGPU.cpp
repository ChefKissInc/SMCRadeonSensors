//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "SMCRadeonGPU.hpp"
#include "KeyImplementations.hpp"
#include <Headers/kern_util.hpp>

OSDefineMetaClassAndStructors(SMCRadeonGPU, IOService);

bool ADDPR(debugEnabled) = true;
uint32_t ADDPR(debugPrintDelay) = 0;

IOService *SMCRadeonGPU::probe(IOService *provider, SInt32 *score) {
    if (!IOService::probe(provider, score)) {
        SYSLOG("smcrgpu", "Failed to probe the parent");
        return nullptr;
    }

    this->fProvider = OSDynamicCast(RadeonSensor, provider);
    if (!this->fProvider) {
        SYSLOG("smcrgpu", "Null fProvider");
        return nullptr;
    }

    auto gpuCount = this->fProvider->getCardCount();
    for (auto i = 0; i < gpuCount; i++) {
        VirtualSMCAPI::addKey(KeyTGxD(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
        VirtualSMCAPI::addKey(KeyTGxP(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
        VirtualSMCAPI::addKey(KeyTGxd(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
        VirtualSMCAPI::addKey(KeyTGxp(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
    }

    return this;
}

bool SMCRadeonGPU::start(IOService *provider) {
    if (!IOService::start(provider)) {
        SYSLOG("init", "Failed to start the parent");
        return false;
    }

    this->setProperty("VersionInfo", xStringify(MODULE_VERSION));

    this->vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    if (!this->vsmcNotifier) {
        SYSLOG("init", "Failed to register notification handler");
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
