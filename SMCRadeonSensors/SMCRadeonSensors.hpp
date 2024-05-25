//  Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5. See LICENSE for
//  details.

#pragma once
#include <Headers/kern_iokit.hpp>
#include <IOKit/IOService.h>
#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>

class EXPORT PRODUCT_NAME : public IOService {
    OSDeclareDefaultStructors(PRODUCT_NAME);

    static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEF") - 1;
    static constexpr const char *KeyIndexes = "0123456789ABCDEF";

    static constexpr SMC_KEY KeyTGxP(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'P'); }
    static constexpr SMC_KEY KeyTGxD(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'D'); }
    static constexpr SMC_KEY KeyTGxp(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'p'); }
    static constexpr SMC_KEY KeyTGxd(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'd'); }
    static constexpr SMC_KEY KeyTGDD = SMC_MAKE_IDENTIFIER('T', 'G', 'D', 'D');

    VirtualSMCAPI::Plugin vsmcPlugin {
        xStringify(PRODUCT_NAME),
        parseModuleVersion(xStringify(MODULE_VERSION)),
        VirtualSMCAPI::Version,
    };

    OSArray *cards {nullptr};
    IONotifier *vsmcNotifier {nullptr};

    public:
    IOService *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void free() APPLE_KEXT_OVERRIDE;
    void stop(IOService *provider) APPLE_KEXT_OVERRIDE;

    virtual UInt16 getTemperature(UInt16 card);
    virtual UInt16 getCardCount();

    static bool vsmcNotificationHandler(void *target, void *refCon, IOService *newService, IONotifier *notifier);
};
