//  Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5. See LICENSE for
//  details.

#include "SMCRadeonSensors.hpp"
#include "KeyImplementations.hpp"
#include "SMCRSCard.hpp"
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
    if (dict == nullptr) {
        SYSLOG("SMCRS", "Failed to create matching dict for IOPCIDevice");
        return nullptr;
    }
    auto *iter = IOService::getMatchingServices(dict);
    dict->release();
    if (iter == nullptr) {
        SYSLOG("SMCRS", "Failed to get iterator over IOPCIDevice");
        return nullptr;
    }
    this->cards = OSArray::withCapacity(0);
    if (this->cards == nullptr) {
        SYSLOG("SMCRS", "Failed to allocate cards array");
        iter->release();
        return nullptr;
    }
    for (auto *device = OSDynamicCast(IOPCIDevice, iter->getNextObject()); device;
         device = OSDynamicCast(IOPCIDevice, iter->getNextObject())) {
        WIOKit::awaitPublishing(device);
        auto vendorID = WIOKit::readPCIConfigValue(device, WIOKit::kIOPCIConfigVendorID);
        auto classCode = (WIOKit::readPCIConfigValue(device, WIOKit::kIOPCIConfigClassCode) >> 8) &
                         WIOKit::ClassCode::PCISubclassMask;
        if (vendorID != WIOKit::VendorID::ATIAMD ||
            (classCode != WIOKit::ClassCode::DisplayController && classCode != WIOKit::ClassCode::VGAController &&
                classCode != WIOKit::ClassCode::Ex3DController && classCode != WIOKit::ClassCode::XGAController)) {
            continue;
        }
        auto *card = new SMCRSCard {};
        if (card == nullptr) { continue; }
        if (card->initialise(device)) { this->cards->setObject(card); }
        card->release();
    }
    iter->release();

    if (this->cards->getCount() == 0) {
        OSSafeReleaseNULL(this->cards);
        return nullptr;
    }

    DBGLOG("SMCRS", "Found %u cards", this->cards->getCount());

    for (UInt32 i = 0; i < this->cards->getCount(); i++) {
        VirtualSMCAPI::addKey(KeyTGxD(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this, i)));
        VirtualSMCAPI::addKey(KeyTGxP(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this, i)));
        VirtualSMCAPI::addKey(KeyTGxd(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this, i)));
        VirtualSMCAPI::addKey(KeyTGxp(i), vsmcPlugin.data,
            VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this, i)));
        if (i == 0) {
            VirtualSMCAPI::addKey(KeyTGDD, vsmcPlugin.data,
                VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new RGPUTempValue(this, i)));
        }
    }

    qsort(const_cast<VirtualSMCKeyValue *>(vsmcPlugin.data.data()), vsmcPlugin.data.size(), sizeof(VirtualSMCKeyValue),
        VirtualSMCKeyValue::compare);

    return this;
}

bool PRODUCT_NAME::start(IOService *provider) {
    if (!IOService::start(provider)) {
        SYSLOG("SMCRS", "Failed to start the parent");
        return false;
    }

    SYSLOG("SMCRS", "Copyright 2023-2024 ChefKiss. If you've paid for this, you've been scammed.");

    this->setProperty("VersionInfo", kextVersion);

    this->vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    if (this->vsmcNotifier == nullptr) {
        SYSLOG("init", "VirtualSMCAPI::registerHandler failed");
        IOService::stop(provider);
        return false;
    }

    return true;
}

bool PRODUCT_NAME::vsmcNotificationHandler(void *target, void *, IOService *newService, IONotifier *) {
    if (target == nullptr || newService == nullptr) {
        SYSLOG("SMCRS", "Null notification");
        return false;
    }

    auto &plugin = static_cast<PRODUCT_NAME *>(target)->vsmcPlugin;
    auto ret = newService->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, target, &plugin, nullptr, nullptr);
    if (ret == kIOReturnSuccess) {
        DBGLOG("SMCRS", "Submitted plugin");
        return true;
    }
    if (ret != kIOReturnUnsupported) {
        SYSLOG("SMCRS", "Plugin submission failure: 0x%X", ret);
        return false;
    }
    SYSLOG("SMCRS", "Plugin submitted to non-VSMC");
    return false;
}

void PRODUCT_NAME::stop(IOService *) { PANIC("SMCRS", "Called stop!!!"); }

void PRODUCT_NAME::free() {
    OSSafeReleaseNULL(this->cards);
    IOService::free();
}

SMCRSCard *PRODUCT_NAME::getCard(UInt32 index) {
    if (this->cards == nullptr || index >= this->cards->getCount()) { return nullptr; }
    return OSDynamicCast(SMCRSCard, this->cards->getObject(index));
}

UInt16 PRODUCT_NAME::getTemperature(UInt32 index) {
    UInt16 temp = 0xFF;
    auto *obj = this->getCard(index);
    if (obj != nullptr) { obj->getTemperature(&temp); }
    return temp;
}

UInt16 PRODUCT_NAME::getCardCount() { return this->cards ? this->cards->getCount() : 0; }

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
    lilu_get_boot_args("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-RSDebug") || checkKernelArgument("-liludbgall");
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) { return KERN_FAILURE; }
