//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RSensor.hpp"
#include "RSensorCard.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>
#include <Headers/kern_iokit.hpp>

void RSensor::init() {
    SYSLOG("rsensor", "Copyright 2023 ChefKiss Inc. If you've paid for this, you've been scammed.");

    lilu.onPatcherLoadForce([](void *user, KernelPatcher &) { static_cast<RSensor *>(user)->populateCards(); }, this);
}

void RSensor::deinit() { OSSafeReleaseNULL(this->cards); }

void RSensor::populateCards() {
    auto *devInfo = DeviceInfo::create();
    if (!devInfo) { return; }
    auto *videoBuiltin = OSDynamicCast(IOPCIDevice, devInfo->videoBuiltin);
    this->cards = OSArray::withCapacity(0);
    if (videoBuiltin &&
        WIOKit::readPCIConfigValue(videoBuiltin, WIOKit::kIOPCIConfigVendorID) == WIOKit::VendorID::ATIAMD) {
        auto *card = new RSensorCard();
        if (card && card->initialise(videoBuiltin)) { this->cards->setObject(card); }
        OSSafeReleaseNULL(card);
    }
    for (size_t i = 0; i < devInfo->videoExternal.size(); i++) {
        auto *obj = OSDynamicCast(IOPCIDevice, devInfo->videoExternal[i].video);
        if (obj && devInfo->videoExternal[i].vendor == WIOKit::VendorID::ATIAMD) {
            auto *card = new RSensorCard();
            if (!card || !card->initialise(obj)) {
                OSSafeReleaseNULL(card);
                continue;
            }
            this->cards->setObject(card);
            card->release();
        }
    }
    DBGLOG("rsensor", "Found %lu cards", this->radeonCards->getCount());
}
