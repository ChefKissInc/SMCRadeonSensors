# Change log

## 2.4.0 (14/11/2025)

### Enhancements

- Cleaned up code and logic.
- Changed debug arg to `-RSDebug`.
- Add power readings for SI, CI, VI, AI.

### Bug Fixes

- Ensure PCI memory space is enabled.
- Call IOService::stop when registerHandler fails.
- Device ID 0x730F is Fiji, not 0x7301.
- Sleep after checking PM_STATUS_95.
- Misc fixes.

**NOTE:** RadeonGadget has been removed. The project is now a single VirtualSMC plug-in. Don't forget to remove the old SMCRadeonGPU/RadeonSensor combination before putting this in. The philosophy behind this is to have as close to native experience as possible. Replacements for RadeonGadget include, but are not limited to, XS-Labs' Hot, exelban's Stats, iStat Menus (paid) and similar.

## 2.3.1 (27/10/2024)

### Enhancements

- Bumped submodules and improved optimisation settings

**NOTE:** RadeonGadget has been removed. The project is now a single VirtualSMC plug-in. Don't forget to remove the old SMCRadeonGPU/RadeonSensor combination before putting this in. The philosophy behind this is to have as close to native experience as possible. Replacements for RadeonGadget include, but are not limited to, XS-Labs' Hot, exelban's Stats, iStat Menus (paid) and similar.

## 2.3.0 (30/09/2024)

### Enhancements

- General project fixes and clean-up.

**NOTE:** RadeonGadget has been removed. The project is now a single VirtualSMC plug-in. Don't forget to remove the old SMCRadeonGPU/RadeonSensor combination before putting this in. The philosophy behind this is to have as close to native experience as possible. Replacements for RadeonGadget include, but are not limited to, XS-Labs' Hot, exelban's Stats, iStat Menus (paid) and similar.

## 2.2.0 (25/05/2024)

### Bug fixes

- On some systems cards would fail to register due to the register MMIO failing to be mapped early.

**NOTE**: RadeonGadget has been removed. The project is now a single VirtualSMC plug-in. Don't forget to remove the old SMCRadeonGPU/RadeonSensor combination before putting this in. The philosophy behind this is to have as close to native experience as possible. Replacements for RadeonGadget include, but are not limited to, XS-Labs' Hot, exelban's Stats, iStat Menus (paid) and similar.

## 2.1.0 (25/05/2024)

### Bug fixes

- Incorrect temperature readings were reported on Polaris and older cards.

**NOTE:** RadeonGadget has been removed. The project is now a single VirtualSMC plug-in. Don't forget to remove the old SMCRadeonGPU/RadeonSensor combination before putting this in. The philosophy behind this is to have as close to native experience as possible. Replacements for RadeonGadget include, but are not limited to, XS-Labs' Hot, exelban's Stats, iStat Menus (paid) and similar.

## 2.0.0 (20/04/2024)

### Enhancements

- RadeonGadget has been removed; the project is now a single VirtualSMC plug-in.

  Don't forget to remove the old SMCRadeonGPU/RadeonSensor combination before putting this in.

  The philosophy behind this is to have as close to native experience as possible.

  Recommended apps for viewing sensors on the system: XS-Labs' Hot, exelban's Stats, iStat Menus (paid), etc.

  Not affiliated with any of these people, just mentioning them for your convenience.

## 1.3.0 (15/09/2023)

### Bug fixes

- Vega20-based dGPUs failed to read temperature.

### Enhancements

- Add Vega12-based GPU support.
- Simplify RMMIO and IOPCIDevice filter logic.

## 1.2.0 (13/09/2023)

### Bug fixes

- Code architecture bug caused SMCRadeonGPU to have no readings.

### Enhancements

- Implement TGDD SMC key.

## 1.1.0 (08/07/2023)

### Bug fixes

- Temperature readings on Navi were broken
- RSensorCard would sometimes fail because RMMIO was mapped too early

## 1.0.0 (24/06/2023)

### Enhancements

- Major code rewrite
- Improve temperature reading logic
- Add AMD iGPU support
- Improve gadget app pop-over design
