//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RadeonSensor.hpp"
#include "RSensorCard.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>
#include <Headers/kern_iokit.hpp>
#include <Headers/kern_version.hpp>

bool ADDPR(debugEnabled) = true;
uint32_t ADDPR(debugPrintDelay) = 0;

OSDefineMetaClassAndStructors(PRODUCT_NAME, IOService);

IOService *PRODUCT_NAME::probe(IOService *provider, SInt32 *score) {
    if (IOService::probe(provider, score) != this) { return nullptr; }

    this->setProperty("VersionInfo", kextVersion);

    auto *dict = IOService::serviceMatching("IOPCIDevice");
    if (!dict) {
        SYSLOG("RSensor", "Failed to create matching dict for IOPCIDevice");
        return nullptr;
    }
    auto *iter = IOService::getMatchingServices(dict);
    dict->release();
    if (!iter) {
        SYSLOG("RSensor", "Failed to get iterator over IOPCIDevice");
        return nullptr;
    }
    this->cards = OSArray::withCapacity(0);
    if (!this->cards) {
        SYSLOG("RSensor", "Failed to allocate cards array");
        iter->release();
        return nullptr;
    }
    for (auto *device = OSDynamicCast(IOPCIDevice, iter->getNextObject()); device;
         device = OSDynamicCast(IOPCIDevice, iter->getNextObject())) {
        WIOKit::awaitPublishing(device);
        UInt32 vendorID = 0, classCode = 0;
        if (!WIOKit::getOSDataValue(device, "vendor-id", vendorID) || vendorID != WIOKit::VendorID::ATIAMD ||
            !WIOKit::getOSDataValue(device, "class-code", classCode) || classCode != WIOKit::ClassCode::VGAController) {
            continue;
        }
        DBGLOG("RSensor", "vendorID: 0x%X", WIOKit::readPCIConfigValue(device, WIOKit::kIOPCIConfigVendorID));
        DBGLOG("RSensor", "classCode: 0x%X", WIOKit::readPCIConfigValue(device, WIOKit::kIOPCIConfigClassCode));
        auto *card = new RSensorCard {};
        if (!card) { continue; }
        if (card->initialise(device)) { this->cards->setObject(card); }
        card->release();
    }
    iter->release();

    if (this->cards->getCount() == 0) {
        OSSafeReleaseNULL(this->cards);
        return nullptr;
    }

    DBGLOG("RSensor", "Found %lu cards", this->cards->getCount());

    return this;
}

bool PRODUCT_NAME::start(IOService *provider) {
    if (!IOService::start(provider)) {
        SYSLOG("RSensor", "Failed to start the parent");
        return false;
    }

    SYSLOG("RSensor", "Copyright 2023 ChefKiss Inc. If you've paid for this, you've been scammed.");

    this->registerService();

    return true;
}

void PRODUCT_NAME::free() {
    OSSafeReleaseNULL(this->cards);
    IOService::free();
}

UInt16 PRODUCT_NAME::getTemperature(UInt16 card) {
    if (!this->cards || card >= this->cards->getCount()) { return 0xFF; }

    auto *obj = OSDynamicCast(RSensorCard, this->cards->getObject(card));
    if (!obj) { return 0xFF; }
    UInt16 temp = 0;
    if (obj->getTemperature(&temp) != kIOReturnSuccess) { return 0xFF; }
    return temp;
}

UInt16 PRODUCT_NAME::getCardCount() { return this->cards ? this->cards->getCount() : 0; }

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
    lilu_get_boot_args("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-rsensordbg") || checkKernelArgument("-liludbgall");
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) { return KERN_FAILURE; }
