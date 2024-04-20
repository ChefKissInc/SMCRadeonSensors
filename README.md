# SMCRadeonSensors ![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/ChefKissInc/SMCRadeonSensors/main.yml?branch=master&logo=github&style=for-the-badge) [![Release Badge](https://img.shields.io/github/release/ChefKissInc/RadeonSensor?include_prereleases&style=for-the-badge&sort=semver&color=blue)](https://github.com/ChefKissInc/SMCRadeonSensors/releases) [![Downloads Badge](https://img.shields.io/github/downloads/ChefKissInc/RadeonSensor/total.svg?style=for-the-badge)](https://github.com/ChefKissInc/SMCRadeonSensors/releases/latest)

A [VirtualSMC](https://github.com/Acidanthera/VirtualSMC) plug-in that provides temperature readings for AMD GPUs.

Supports AMD dGPUs starting with the `Radeon HD 7000` series up to the `RX 6000 series` and all `Vega` (GCN 5/Raven ASIC) iGPUs.

Requires `macOS 10.14` or newer, [Lilu](https://github.com/Acidanthera/Lilu) and [VirtualSMC](https://github.com/Acidanthera/VirtualSMC).

The Source Code of this Original Work is licensed under the `Thou Shalt Not Profit License version 1.0`. See [`LICENSE`](https://github.com/NootInc/NootedRed/blob/master/LICENSE).

## AMD GPU temperature and macOS

Apple stopped reporting the temperature since the `Radeon VII`. `Vega 10` and older can still report the temperature using external tools.

### Credits

* [Linux](https://github.com/torvalds/linux) for the temperature logic in the `amdgpu` driver
* [Acidanthera](https://github.com/Acidanthera) for `Lilu` and `VirtualSMC`
* [AMD](https://amd.com) for their GPUs.
* [Aluveitie](https://github.com/Aluveitie) for the previous work.
* [FakeSMC3](https://github.com/CloverHackyColor/FakeSMC3_with_plugins) for the original code, adapted by Aluveitie for `VirtualSMC`, now rewritten by us.
