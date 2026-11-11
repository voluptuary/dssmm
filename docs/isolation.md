# Hardware project isolation

The default `dssmm` branch owns three independent projects. The standalone `ezfomegade`, `ezfomega` and `ezfblue` branches keep their respective root build environments; they are not merge sources for the canonical repository layout.

| Hardware | Canonical project | Standalone verification |
| --- | --- | --- |
| Definitive Edition Rev. A/B | targets/ezfomegade | scripts/verify-ezfomegade.sh |
| Original OMEGA | targets/ezfomega | scripts/verify-omega-port.sh |
| Newer Definitive Edition B / BLUE | targets/ezfblue | scripts/verify-blue-port.sh |

Each project owns its source, assets, tools, tests, settings, documentation and historical records. BLUE preserves its vendor linker/resources subproject internally. No mutable source/asset symlinks connect projects. Root contains only shared orchestration/environment, licensing/provenance and overview/audit records. `projects.json` pins imported commits; there is no automatic branch synchronization.

## Validation evidence

All cleaned standalone target verifiers and all relocated canonical target verifiers passed. Snapshot source/tool/asset files match their pinned Git blobs; declared Windows batch EOL attributes are respected. Local AGENTS/HANDOFF/PROGRESS overlays record canonical ownership. Use `python3 scripts/check-snapshots.py` to check imported provenance, and `make check-isolation` plus `python3 tests/test-project-isolation.py` to test layout/PR boundaries.

[Binary comparisons](relocation-binary-verification.json) show Original OMEGA and packed BLUE updates are byte-identical to the published test builds. DE's executable payload is byte-identical to8.3.2; its default directory-derived GBA header title and corresponding valid checksum differ after relocation. No firmware C/assembly, graphics, FPGA resources or hardware protocols changed. [Snapshot records](snapshot-source-verification.json) list source commits and checked files.

The root shared site is composed with `python3 scripts/build-site.py` into ignored `.site/` from universal pages and project-private documentation. Pages deployment is manual and restricted to dssmm; a hardware branch cannot overwrite the shared site through its workflow.

## Commits, pushes and pull requests

- A push advances only its explicitly named branch. No workflow/script pushes another branch, merges platform branches, synchronizes sources or publishes a kernel release. CI has only `contents: read` and checkout credentials are not persisted.
- Push/PR checks choose hardware from the branch or PR base: one target on a hardware branch, all three projects on dssmm. Historical test baselines require a full Git checkout; CI fetches full history/tags.
- Direct known hardware-branch PRs into another hardware branch or dssmm are rejected. A whole canonical-layout PR into a standalone branch is also rejected. Adapt intended fixes on a feature branch based on the correct destination, within its matching project.
- Policy tests reject mixed identities, foreign root/project directories, misplaced root hardware tools and cross-project source symlinks. Tests/builds can detect regressions; they do not prove that every future edit or merge will be safe.

[GitHub observations](branch-protection-observation.json) show all four branches were unprotected at inspection. These CI checks are therefore not mandatory server-side protection against arbitrary direct pushes, skipped/edited workflows or merges made despite failures. Required checks/reviews must be configured through repository settings by an authorized administrator if enforcement is desired. No protection gate was bypassed or unrequested review requirement imposed.

Initial hosted runs failed on container/checkout UID ownership in historical Git fixtures, after successful kernel builds. The corrected workflow trusts only the exact GITHUB_WORKSPACE, never a wildcard. BLUE now keeps real Git history rooted in the checkout while its copied fixtures/headers use only actual BLUE source. The final CI disposition is recorded in HANDOFF/PROGRESS.

## Remaining limitations

Hardware validation remains model-specific. Original OMEGA and BLUE still have no physical qualification; BLUE larger-image updater acceptance remains unverified. Static RAM gaps are not runtime stack peaks. Existing flash polling, power-loss metadata limits, rare NES layouts and large-cheat parsing limits remain in each project's audit documentation. Structural cleanup is not a new kernel release; all published tags/assets remain immutable.
