// Copyright © 2023-2024 ChefKiss. Licensed under the Thou Shalt Not Profit License version 1.5.
// See LICENSE for details.

#pragma once
#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>

enum struct SMCRSChipFamily {
    Unknown,
    SeaIslands,
    SouthernIslands,
    VolcanicIslands,
    ArcticIslands,
    Raven,
    Navi,
};

#ifdef DEBUG
inline const char *stringifyChipFamily(SMCRSChipFamily value) {
    switch (value) {
        case SMCRSChipFamily::SeaIslands:
            return "Sea Islands";
        case SMCRSChipFamily::SouthernIslands:
            return "Southern Islands";
        case SMCRSChipFamily::VolcanicIslands:
            return "Volcanic Islands";
        case SMCRSChipFamily::ArcticIslands:
            return "Arctic Islands";
        case SMCRSChipFamily::Raven:
            return "Raven";
        case SMCRSChipFamily::Navi:
            return "Navi";
        default:
            return "Unknown";
    }
}
#endif

class SMCRSAttributes {
    static constexpr UInt16 ChipFamilyMask = 7U;
    static constexpr UInt16 IsTHM11 = (1U << 3);
    static constexpr UInt16 SupportsPower = (1U << 4);

    UInt16 value {0};

    public:
    inline SMCRSChipFamily getFamily() const { return static_cast<SMCRSChipFamily>(this->value & ChipFamilyMask); }
    inline bool isTHM11() const { return (this->value & IsTHM11) != 0; }
    inline bool supportsPower() const { return (this->value & SupportsPower) != 0; }

    inline void setFamily(SMCRSChipFamily value) { this->value |= static_cast<UInt16>(value); }
    inline void setIsTHM11() { this->value |= IsTHM11; }
    inline void setSupportsPower() { this->value |= SupportsPower; }
};

class SMCRSCard : public OSObject {
    OSDeclareDefaultStructors(SMCRSCard);

    IOPCIDevice *dev {nullptr};
    volatile UInt32 *rmmioPtr {nullptr};
    IOMemoryMap *rmmio {nullptr};
    SMCRSAttributes attributes {};
    IOLock *regLock {nullptr};
    IOLock *indLock {nullptr};
    IOLock *smcLock {nullptr};

    IOReturn ensureRMMIOMapped();
    UInt32 readReg32(UInt32 reg);
    void writeReg32(UInt32 reg, UInt32 val);
    UInt32 soc15ReadReg32(UInt32 reg);
    void soc15WriteReg32(UInt32 reg, UInt32 val);
    UInt32 readIndirectSMC(UInt32 reg);
    void writeIndirectSMC(UInt32 reg, UInt32 val);
    UInt32 smu7WaitForSMCResp();
    UInt32 smu7SendMessageToSMC(UInt32 message, UInt32 param = 0, UInt32 *outData = nullptr);
    UInt32 smu9WaitForSMCResp();
    UInt32 smu9SendMessageToSMC(UInt32 message, UInt32 param = 0, UInt32 *outData = nullptr);

    IOReturn smu7GetTemp(UInt16 *data);
    IOReturn thm9GetTemp(UInt16 *data);
    IOReturn thm11GetTemp(UInt16 *data);
    IOReturn thm10GetTemp(UInt16 *data);

    IOReturn smu7GetPowerPMStatus(float *data);
    IOReturn smu7GetPowerSMC(float *data);
    IOReturn smu9GetPower(float *data);

    public:
    bool initialise(IOPCIDevice *device);
    IOReturn getTemperature(UInt16 *data);
    bool supportsPower();
    IOReturn getPower(float *data);
};
