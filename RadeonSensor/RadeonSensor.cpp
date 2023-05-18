//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "RadeonSensor.hpp"
#include <Headers/kern_util.hpp>
#include <IOKit/IOLib.h>

#define super IOService

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

OSDefineMetaClassAndStructors(RadeonSensor, IOService);

bool RadeonSensor::init(OSDictionary *dictionary) {
    if (!super::init(dictionary)) {
        IOLog("RadeonSensor::init: super::init failed\n");
        return false;
    }

    IOLog("RadeonSensor::init\n");

    return true;
}

void RadeonSensor::free() {
    IOLog("RadeonSensor::free\n");

    delete[] radeonCards;

    super::free();
}

IOService *RadeonSensor::probe(IOService *provider, SInt32 *score) {
    if (super::probe(provider, score) != this) { return nullptr; }

    int count = 0, maxCount = 4;    // 4 GPUs max
    RadeonCard **cards = new RadeonCard *[4];
    if (OSDictionary *dictionary = serviceMatching("IOPCIDevice")) {
        if (OSIterator *iterator = getMatchingServices(dictionary)) {
            UInt32 vendor_id = 0, class_id = 0;
            IOPCIDevice *device = nullptr;
            do {
                device = OSDynamicCast(IOPCIDevice, iterator->getNextObject());
                if (!device) { break; }
                vendor_id = 0;
                OSData *data = OSDynamicCast(OSData, device->getProperty("vendor-id"));
                if (data) {
                    vendor_id = *(UInt32 *)data->getBytesNoCopy();
                } else {
                    data = OSDynamicCast(OSData, device->getProperty("ATY,VendorID"));
                    if (data) { vendor_id = *(UInt32 *)data->getBytesNoCopy(); }
                }

                int device_id = 0;
                data = OSDynamicCast(OSData, device->getProperty("device-id"));
                if (data) { device_id = *(UInt32 *)data->getBytesNoCopy(); }

                class_id = 0;
                data = OSDynamicCast(OSData, device->getProperty("class-code"));
                if (data) { class_id = *(UInt32 *)data->getBytesNoCopy(); }

                if ((vendor_id == 0x1002) && (class_id == 0x030000)) {
                    IOLog("RadeonSensor found Radeon PCIe device id=%x", (unsigned int)device_id);
                    RadeonCard *radeonCard = new RadeonCard();
                    if (radeonCard->initialise(device, device_id)) {
                        IOLog("RadeonSensor initialized card (%x)", device_id);
                        cards[count] = radeonCard;
                        count++;
                    } else {
                        IOLog("RadeonSensor could not initialize card (%x)", device_id);
                    }

                    if (count == maxCount) { break; }
                }
            } while (device);
        }
    }

    if (count > 0) {
        this->cardCount = count;
        this->radeonCards = cards;
        return this;
    }

    return nullptr;
}

bool RadeonSensor::start(IOService *provider) {
    if (!super::start(provider)) {
        IOLog("RadeonSensor::start: super::start failed\n");
        return false;
    }

    this->registerService();

    IOLog("RadeonSensor::start\n");
    return true;
}

void RadeonSensor::stop(IOService *provider) {
    IOLog("RadeonSensor::stop\n");

    super::stop(provider);
}

UInt16 RadeonSensor::getTemperature(UInt16 card) {
    if (card >= this->cardCount) { return 0xFF; }

    UInt16 temp = 0;
    this->radeonCards[card]->getTemperature(&temp);
    return temp;
}

UInt16 RadeonSensor::getCardCount() { return cardCount; }

EXPORT extern "C" kern_return_t radeonsensor_start(kmod_info_t *, void *) {
    // Report success but actually do not start and let I/O Kit unload us.
    // This works better and increases boot speed in some cases.
    PE_parse_boot_argn("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-radpdbg");

    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t radeonsensor_stop(kmod_info_t *, void *) {
    // It is not safe to unload VirtualSMC plugins!
    return KERN_FAILURE;
}
