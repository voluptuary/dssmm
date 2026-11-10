# Merkin Mix port exploration

ezfomega is now in active implementation under owner authorization; see OMEGA_STATUS.md. ezfblue remains preparation only. No new target has hardware validation yet.

| Branch | Target | Reference repository | Pinned reference commit |
| --- | --- | --- | --- |
| ezfode | Existing Omega Definitive Edition Rev. A/B | Current repository | Source implementation 9944dc7 |
| ezfomega | Original EZ-FLASH OMEGA | https://github.com/FrankieT19/omega-ds-style-kernel | ef30b8ba78147943d95653dfa82f721b52096f67 |
| ezfblue | Omega Definitive Edition BLUE, as named by vendor source | https://github.com/ezflash-team/blue-kernel | 109f1a050d9c35008379ce61ae40d05dc528c614 |

Both exploration branches start from the same Merkin Mix baseline. Their inherited source and Makefile still target Omega DE Rev. A/B. Branch names do not make these sources suitable for other cartridges. Each branch adds TARGET.md with its own exploration plan.

## Resume procedure

1. Read AGENTS.md, HANDOFF.md, PROGRESS.md, and the branch TARGET.md.
2. Re-fetch the target reference into a dedicated reference ref and verify the pinned commit. Do not merge upstream wholesale or infer code novelty from release numbers.
3. Inventory hardware mappings, storage/save behavior, launch and reset protocols, IRQ/DMA ownership, RAM layout, image size limits and update packaging before selecting a target source baseline.
4. Compare final Merkin Mix behavior with the target baseline. Record features already present, actual missing changes, dependencies, original authors and licenses. Preserve FrankieT19 and other contributors' attribution.
5. Plan small changes with host regression coverage and a target-specific hardware test matrix. Reuse pure policy/UI code only after checking dependencies. Keep hardware operations target-specific.
6. Wait for the owner to resume actual porting before implementing changes, building target candidates, or publishing releases.

## Initial feature inventory to reconcile

- Transparent and name-matched artwork across supported systems, opaque header-art acceleration, cache lifetime and rendering improvements.
- Browser/navigation, themes, settings, remembered cheats, file identity and FatFs lookup improvements.
- Save/settings transaction safeguards and recovery limitations; loader bounds, exact I/O, patch/window boundaries and memory layout guards.
- Known unresolved NOR timing/allocation findings F-022/F-032: target-specific re-evaluation required, without guessed timeouts or spare sectors.

## Validation state

Only reference README/build metadata and repository trees have been inspected. Both references advertise Apache-2.0 metadata; detailed file-level provenance remains to be reviewed. No feasibility conclusion, target build, hardware validation or port implementation has occurred.
