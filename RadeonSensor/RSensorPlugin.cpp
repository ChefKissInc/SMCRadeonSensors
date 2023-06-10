//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RSensor.hpp"
#include "RSensorCard.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_version.hpp>
#include <Headers/plugin_start.hpp>

static RSensor rsensor;

static const char *bootargOff[] = {
    "-rsensoroff",
};

static const char *bootargDebug[] = {
    "-rsensordbg",
};

static const char *bootargBeta[] = {
    "-rsensorbeta",
};

PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal | LiluAPI::AllowInstallerRecovery | LiluAPI::AllowSafeMode,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
    KernelVersion::Catalina,
    KernelVersion::Sonoma,
    []() { rsensor.init(); },
};

OSDefineMetaClassAndStructors(PRODUCT_NAME, IOService);

IOService *PRODUCT_NAME::probe(IOService *provider, SInt32 *score) {
    setProperty("VersionInfo", kextVersion);
    auto service = IOService::probe(provider, score);
    return ADDPR(startSuccess) ? service : nullptr;
}

bool PRODUCT_NAME::start(IOService *provider) {
    if (!IOService::start(provider)) {
        SYSLOG("init", "Failed to start the parent");
        return false;
    }

    this->registerService();

    return ADDPR(startSuccess);
}

void PRODUCT_NAME::free() {
    rsensor.deinit();
    IOService::free();
}

UInt16 PRODUCT_NAME::getTemperature(UInt16 card) {
    if (!rsensor.cards || card >= rsensor.cards->getCount()) { return 0xFF; }

    auto *obj = OSDynamicCast(RSensorCard, rsensor.cards->getObject(card));
    if (!obj) { return 0xFF; }
    UInt16 temp = 0;
    obj->getTemperature(&temp);
    return temp;
}

UInt16 PRODUCT_NAME::getCardCount() { return rsensor.cards ? rsensor.cards->getCount() : 0; }
