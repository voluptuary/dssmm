# Merkin Mix port exploration

BLUE implementation has resumed on ezfblue. See [BLUE_STATUS.md](BLUE_STATUS.md) for the active baseline and validation limits. Omega has a separate hardware-untested prerelease; no cross-target hardware compatibility is implied.

| Branch | Target | Reference repository | Pinned reference commit |
| --- | --- | --- | --- |
| ezfode | Existing Omega Definitive Edition Rev. A/B | Current repository | Source implementation 9944dc7 |
| ezfomega | Original EZ-FLASH OMEGA | https://github.com/FrankieT19/omega-ds-style-kernel | ef30b8ba78147943d95653dfa82f721b52096f67 |
| ezfblue | Omega Definitive Edition BLUE, as named by vendor source | https://github.com/ezflash-team/blue-kernel | 109f1a050d9c35008379ce61ae40d05dc528c614 |

Both exploration branches began at the same Merkin Mix baseline. On ezfblue, root source/images remain DE donor material and the active root build now delegates only to targets/blue. Each branch maintains its own target plan; branch names alone do not establish compatibility.

## Resume procedure

1. Read AGENTS.md, HANDOFF.md, PROGRESS.md, and the branch TARGET.md.
2. Re-fetch the target reference into a dedicated reference ref and verify the pinned commit. Do not merge upstream wholesale or infer code novelty from release numbers.
3. Inventory hardware mappings, storage/save behavior, launch and reset protocols, IRQ/DMA ownership, RAM layout, image size limits and update packaging before selecting a target source baseline.
4. Compare final Merkin Mix behavior with the target baseline. Record features already present, actual missing changes, dependencies, original authors and licenses. Preserve FrankieT19 and other contributors' attribution.
5. Plan small changes with host regression coverage and a target-specific hardware test matrix. Reuse pure policy/UI code only after checking dependencies. Keep hardware operations target-specific.
6. BLUE porting is authorized. Implement and validate small target-specific changes; do not publish a BLUE release or claim hardware verification without supporting evidence.

## Initial feature inventory to reconcile

- Transparent and name-matched artwork across supported systems, opaque header-art acceleration, cache lifetime and rendering improvements.
- Browser/navigation, themes, settings, remembered cheats, file identity and FatFs lookup improvements.
- Save/settings transaction safeguards and recovery limitations; loader bounds, exact I/O, patch/window boundaries and memory layout guards.
- Known unresolved NOR timing/allocation findings F-022/F-032: target-specific re-evaluation required, without guessed timeouts or spare sectors.

## Validation state

The initial preparation record is superseded for BLUE by BLUE_STATUS.md. Its vendor baseline builds after declaration/case fixes; portable resource packaging has host tests. The DS Style launcher still needs integration and no BLUE hardware validation has occurred. Component notices remain in the imported source.
