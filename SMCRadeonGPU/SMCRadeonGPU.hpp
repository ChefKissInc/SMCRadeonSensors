//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef SMCRadeonGPU_hpp
#define SMCRadeonGPU_hpp

#include <kern_rsensor.hpp>
#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>

class EXPORT SMCRadeonGPU : public IOService {
    OSDeclareDefaultStructors(SMCRadeonGPU);

    static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEF") - 1;
    static constexpr const char *KeyIndexes = "0123456789ABCDEF";

    static constexpr SMC_KEY KeyTGxP(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'P'); }
    static constexpr SMC_KEY KeyTGxD(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'D'); }
    static constexpr SMC_KEY KeyTGxp(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'p'); }
    static constexpr SMC_KEY KeyTGxd(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'd'); }

    VirtualSMCAPI::Plugin vsmcPlugin {
        xStringify(PRODUCT_NAME),
        parseModuleVersion(xStringify(MODULE_VERSION)),
        VirtualSMCAPI::Version,
    };

    public:
    IOService *probe(IOService *provider, SInt32 *score) override;

    bool start(IOService *provider) override;
    void stop(IOService *provider) override;

    IOReturn setPowerState(unsigned long state, IOService *whatDevice) override;

    static bool vsmcNotificationHandler(void *target, void *refCon, IOService *newService, IONotifier *notifier);

    private:
    RadeonSensor *fProvider {nullptr};
    IONotifier *vsmcNotifier {nullptr};
};

#endif /* SMCRadeonGPU_hpp */
