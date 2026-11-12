# ezfblue — current working architecture

Target: EZ-FLASH OMEGA Definitive Edition B / BLUE. Repository: voluptuary/dssmm.
Read root AGENTS.md plus this directory's HANDOFF.md and PROGRESS.md first.

All four branches track the same combined tree. This hardware branch is a
development lane for ezfblue/, optionally shown through worktree-local sparse
checkout. Other project directories remain tracked even when hidden. Never
commit their removal. Kernel source/assets/tests/protocols stay private here.

On branch ezfblue, edit only this directory and update THIS AGENTS/HANDOFF/
PROGRESS as needed. Root journals are for shared architecture/integration only.
Merge ezfblue normally into dssmm, then merge dssmm back to synchronize.
Do not directly merge a different hardware lane; integrate through dssmm.
Universal root changes belong on dssmm or a shared feature branch.

From the Git root, run make ezfblue and make verify-ezfblue. A sparse view
can be set with git sparse-checkout set --cone .devcontainer .github docs scripts
tests ezfblue; disable sparse checkout for the full combined view. See root
docs/isolation.md for complete commands, CI policy and validation limits.

Previous agent instructions and exact branch journals are preserved under
docs/history/pre-shared-tree/; earlier single-project tracked-tree, frozen
snapshot and blanket merge prohibitions are historical, superseded by this
architecture. Current project HANDOFF/PROGRESS preserve their complete history.

Preserve hardware-specific flash/save/reset protocols, all credits and logs.
Source/build/host checks do not qualify physical hardware. For the authorized 8.4.0 publication, DE is stable; OMEGA Test4 and BLUE Test3
remain hardware-untested prereleases. New MiGoomba integration needs hardware testing.
Do not recreate candidates, replace releases or upload manual source ZIPs for
repository maintenance. Never claim arbitrary future same-file changes cannot
conflict; test merges and builds relevant to the actual change.

## MiGoomba integration

GB/GBC payload is MiGoomba v0.1 at4b401e2, variant generic.
See docs/plugins/MIGOOMBA.md and migoomba.json for exact source/digests, variant
limitations and reproduction. Do not edit the generated migoomba.h manually or
reuse another card's variant. Run the focused payload/loader test and full
project verifier for changes; preserve64KiB .esv handling and cache bounds.
Do not claim old save-state compatibility or hardware qualification from mocks.
DSSMM 8.4.0 includes MiGoomba; earlier releases retain their previous emulator.

## BLUE development after 8.4.0

Read docs/porting/FRANKIET19_BLUE_COMPARISON.md for the pinned upstream review.
The 8.4.1-blue-dev build is firmware-free, contiguous and sealed at the stock
2124252-byte update size. Use pack-blue-kernel.py with --elf; no active vendor
resource insertion or firmware updater. Historical resource fixtures stay.
Do not enable persistent NOR RTS without adapting/validating the matching runtime
and SD sector-map behavior. Upstream Test5 and this integration remain physically
unverified. Preserve the published 8.4.0 Test3 artifact and its evidence.
