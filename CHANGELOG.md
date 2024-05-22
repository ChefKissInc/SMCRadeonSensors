# Change log

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
