# DS Style Merkin Mix — BLUE port development

This branch is beginning the Merkin Mix port for the **EZ-FLASH OMEGA Definitive Edition B / BLUE** using EZ-FLASH's published BLUE kernel source.

**Development only, hardware untested. No BLUE test release is available.** The initial build uses the vendor's classic menu while the DS Style launcher and Merkin Mix features are adapted. It is not yet a finished DS Style port.

The stable DE Rev. A/B project remains on [ezfode](https://github.com/voluptuary/ezfode-dssmm-kernel/tree/ezfode). Original Omega work is separate on [ezfomega](https://github.com/voluptuary/ezfode-dssmm-kernel/tree/ezfomega). Neither port changes ezfode.

Run `make` with devkitPro/devkitARM and libgba installed. Only `targets/blue` is compiled; root `source/` and `images/` are retained as DS Style donor material. The raw development GBA image needs the BLUE resource image and is **not a standalone kernel update**. See [BLUE_STATUS.md](docs/porting/BLUE_STATUS.md) for packaging, validation, missing features, and next steps.

Merkin Mix builds on FrankieT19's DS Style work and the EZ-FLASH/Sterophonick kernel lineage. This port preserves the vendor BLUE hardware baseline and existing contributor notices. See [CREDITS.md](CREDITS.md) and [the BLUE baseline](targets/blue/README.md).
