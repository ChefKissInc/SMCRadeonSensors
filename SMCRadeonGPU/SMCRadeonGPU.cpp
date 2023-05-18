//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "SMCRadeonGPU.hpp"
#include "Headers/kern_util.hpp"
#include "KeyImplementations.hpp"
#include <os/log.h>

OSDefineMetaClassAndStructors(SMCRadeonGPU, IOService);

bool SMCRadeonGPU::init(OSDictionary *dictionary) {
    if (!IOService::init(dictionary)) { return false; }

    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::init");
    return true;
}

IOService *SMCRadeonGPU::probe(IOService *provider, SInt32 *score) {
    auto ptr = IOService::probe(provider, score);
    if (!ptr) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::probe: super::probe failed");
        return nullptr;
    }

    this->fProvider = OSDynamicCast(RadeonSensor, provider);
    if (!this->fProvider) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::init: null fProvider");
        return nullptr;
    }

    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::init: setting up SMC keys");
    auto gpuCount = this->fProvider->getCardCount();
    bool suc = true;
    for (auto i = 0; i < gpuCount; i++) {
        suc &= VirtualSMCAPI::addKey(KeyTGxD(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
        suc &= VirtualSMCAPI::addKey(KeyTGxP(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
        suc &= VirtualSMCAPI::addKey(KeyTGxd(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
        suc &= VirtualSMCAPI::addKey(KeyTGxp(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this->fProvider, i)));
    }

    if (!suc) { os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::init: setting up SMC keys failed"); }

    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::probe");
    return this;
}

void SMCRadeonGPU::free(void) { os_log(OS_LOG_DEFAULT, "SMCRadeonGPU freeing up"); }

bool SMCRadeonGPU::start(IOService *provider) {
    if (!IOService::start(provider)) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::start: super::start failed");
        return false;
    }

    this->setProperty("VersionInfo", xStringify(MODULE_VERSION));

    this->vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    if (!this->vsmcNotifier) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::start: failed to register vsmc notification handler");
        return false;
    }

    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU::start");
    return true;
}

bool SMCRadeonGPU::vsmcNotificationHandler(void *target, [[maybe_unused]] void *refCon, IOService *newService,
    [[maybe_unused]] IONotifier *notifier) {
    if (!target || !newService) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU null vsmc notification");
        return false;
    }

    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU got vsmc notification");
    auto &plugin = static_cast<SMCRadeonGPU *>(target)->vsmcPlugin;
    auto ret = newService->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, target, &plugin, nullptr, nullptr);
    if (ret == kIOReturnSuccess) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU submitted plugin");
        return true;
    } else if (ret != kIOReturnUnsupported) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU plugin submission failure %X", ret);
        return false;
    } else {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU plugin submission to non vsmc");
        return false;
    }
}

void SMCRadeonGPU::stop([[maybe_unused]] IOService *provider) { PANIC("smcrgpu", "called stop!!!"); }
