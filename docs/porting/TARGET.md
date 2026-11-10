# Omega Definitive Edition BLUE exploration

Branch: `ezfblue`

Framework baseline: `691dc5ec0391cd30c537082a8428978b29c5500a`

Reference: https://github.com/ezflash-team/blue-kernel

Pinned commit: `109f1a050d9c35008379ce61ae40d05dc528c614`

Status: preparation only; no source port, build-system change, candidate or hardware validation. Inherited source remains the DE Rev. A/B implementation.

## First investigation when resumed

Start with the vendor memory/register map, flash/save protocols and boot/update format. The vendor README requires linking the compiled image with image.bin using Link_kernel_image.exe; inspect that packaging contract and tool reproducibility before designing a build. Establish supported board identity and limits from source evidence.

Follow [the shared preparation checklist](README.md). Produce a feature/provenance matrix and hardware/build inventory before changing code. Porting is paused until the owner resumes it.
