# Omega Definitive Edition BLUE exploration

Branch: `ezfblue`

Framework baseline: `691dc5ec0391cd30c537082a8428978b29c5500a`

Reference: https://github.com/ezflash-team/blue-kernel

Pinned commit: `109f1a050d9c35008379ce61ae40d05dc528c614`

Status: owner resumed implementation. Hardware/build inventory is in [BLUE_STATUS.md](BLUE_STATUS.md). Establish a vendor BLUE build under targets/blue and selectively adapt DS Style; no hardware validation or public BLUE candidate.

## First investigation when resumed

Start with the vendor memory/register map, flash/save protocols and boot/update format. The vendor README requires linking the compiled image with image.bin using Link_kernel_image.exe; inspect that packaging contract and tool reproducibility before designing a build. Establish supported board identity and limits from source evidence.

Follow [the shared preparation checklist](README.md). Produce a feature/provenance matrix and hardware/build inventory before changing code. The owner has resumed BLUE porting; keep all work isolated on ezfblue.
