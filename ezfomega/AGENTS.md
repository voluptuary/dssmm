# ezfomega — current working architecture

Always work in the most token-efficient manner possible: reuse existing findings and validation evidence, read only relevant context, batch independent checks, and avoid duplicate work or unnecessary output. Preserve correctness, required testing, and clear continuation notes.

Target: EZ-FLASH OMEGA (original). Repository: voluptuary/dssmm.
Read root `AGENTS.md` and `docs/AGENT-QUICKSTART.md`, then this directory's
compact `HANDOFF.md`; use `PROGRESS.md` for recent entries. Pre-compaction
journals are archived under `docs/history/journal-pre-compaction-2026-11-26/`.

All five branches (`dssmm`, `ezfomegade`, `ezfomega`, `ezfblue`, `superfw`) track the same combined tree with four isolated project directories. This hardware branch is a
development lane for ezfomega/, optionally shown through worktree-local sparse
checkout. Other project directories remain tracked even when hidden. Never
commit their removal. Kernel source/assets/tests/protocols stay private here.

On branch ezfomega, edit only this directory and update THIS AGENTS/HANDOFF/
PROGRESS as needed. Root journals are for shared architecture/integration only.
Merge ezfomega normally into dssmm, then merge dssmm back to synchronize.
Do not directly merge a different hardware lane; integrate through dssmm.
Universal root changes belong on dssmm or a shared feature branch.

From the Git root, run make ezfomega and make verify-ezfomega. A sparse view
can be set with git sparse-checkout set --cone .devcontainer .github docs scripts
tests ezfomega; disable sparse checkout for the full combined view. See root
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

GB/GBC payload is MiGoomba v0.1 at4b401e2, variant ezo.
See docs/plugins/MIGOOMBA.md and migoomba.json for exact source/digests, variant
limitations and reproduction. Do not edit the generated migoomba.h manually or
reuse another card's variant. Run the focused payload/loader test and full
project verifier for changes; preserve64KiB .esv handling and cache bounds.
Do not claim old save-state compatibility or hardware qualification from mocks.
DSSMM 8.4.0 includes MiGoomba; earlier releases retain their previous emulator.

2026-11-24 current prerelease: v8.4.1-omega-test5/source29bbb05, fixes clipped alias-free SD launch names. Full local/source/all4 native mGBA checks passed; physical hardware untested. Evidence under docs/releases/. Keep Original OMEGA drivers and hardware limits isolated.
