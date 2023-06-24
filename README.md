# RadeonSensor ![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/NootInc/RadeonSensor/main.yml?branch=master&logo=github&style=for-the-badge) [![Release Badge](https://img.shields.io/github/release/NootInc/RadeonSensor?include_prereleases&style=for-the-badge&sort=semver&color=blue)](https://github.com/NootInc/RadeonSensor/releases) [![Downloads Badge](https://img.shields.io/github/downloads/NootInc/RadeonSensor/total.svg?style=for-the-badge)](https://github.com/NootInc/RadeonSensor/releases/latest "Download status")

macOS Kext and Gadget that display the temperature of AMD GPUs in the system.

Supports AMD dGPUs starting with the `Radeon HD 7000` series up to the `RX 6000 series` and all `Vega` (GCN 5/Raven ASIC) iGPUs.

Requires `macOS 10.15` and newer.

The Source Code of this Original Work is licensed under the `Thou Shalt Not Profit License version 1.0`. See [`LICENSE`](https://github.com/NootInc/NootedRed/blob/master/LICENSE).

![Status Bar Screenshot](Assets/StatusBar.png)

## AMD GPU temperature and macOS

Apple stopped reporting the temperature since the `Radeon VII`. `Vega 10` and older can still report the temperature using external tools.

## Components

* `RadeonSensor.kext`: Main sensor component. Requires `Lilu`.
* `SMCRadeonGPU.kext`: Exports sensor values to VirtualSMC for monitoring tools that utilise the SMC.
* `RadeonGadget.app`: Displays GPU temperature in the status bar. Requires `RadeonSensor`.

### Credits

* [Linux](https://github.com/torvalds/linux) for the temperature logic in the `amdgpu` driver
* [Acidanthera](https://github.com/Acidanthera) for `Lilu` and `VirtualSMC`
* [AMD](https://amd.com) for their GPUs.
* [Aluveitie](https://github.com/Aluveitie) for the previous work.
* [FakeSMC3](https://github.com/CloverHackyColor/FakeSMC3_with_plugins) for the original code, adapted by Aluveitie for `VirtualSMC`, now rewritten by us.
