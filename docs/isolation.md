# Hardware project isolation

The default `dssmm` branch owns three independent projects. The `ezfomegade`, `ezfomega` and `ezfblue` branches use the same top-level project paths and universal root environment, each containing only its own hardware project.

| Hardware | Canonical project | Project-local verification |
| --- | --- | --- |
| Definitive Edition Rev. A/B | ezfomegade | scripts/verify-ezfomegade.sh |
| Original OMEGA | ezfomega | scripts/verify-omega-port.sh |
| Newer Definitive Edition B / BLUE | ezfblue | scripts/verify-blue-port.sh |

Each project owns its source, assets, tools, tests, settings, documentation and historical records. BLUE preserves its vendor linker/resources subproject internally. No mutable source/asset symlinks connect projects. Root contains only shared orchestration/environment, licensing/provenance and overview/audit records. `projects.json` pins imported commits; there is no automatic branch synchronization.

## Validation evidence

The dedicated hardware branch verifiers pass from their matching top-level directories; dssmm runs the same project verifiers. Snapshot source/tool/asset files match their pinned Git blobs; declared Windows batch EOL attributes are respected. Local AGENTS/HANDOFF/PROGRESS overlays record canonical ownership. Use `python3 scripts/check-snapshots.py` to check imported provenance, and `make check-isolation` plus `python3 tests/test-project-isolation.py` to test layout/PR boundaries.

[Current binary comparisons](https://github.com/voluptuary/ezfode-dssmm-kernel/blob/dssmm/docs/top-level-binary-verification.json) show all three builds on dssmm and on their dedicated hardware branches are byte-identical to the downloaded published releases. DE now explicitly preserves its published GBA header title, making its complete image independent of the checkout directory. The earlier relocation report documents the superseded directory-derived title difference. No firmware C/assembly, graphics, FPGA resources or hardware protocols changed. [Snapshot records](https://github.com/voluptuary/ezfode-dssmm-kernel/blob/dssmm/docs/snapshot-source-verification.json) list source commits and checked files.

The root shared site is composed with `python3 scripts/build-site.py` into ignored `.site/` from universal pages and project-private documentation. Pages deployment is manual and restricted to dssmm; a hardware branch cannot overwrite the shared site through its workflow.

## Commits, pushes and pull requests

- A push advances only its explicitly named branch. No workflow/script pushes another branch, merges platform branches, synchronizes sources or publishes a kernel release. CI has only `contents: read` and checkout credentials are not persisted.
- Push/PR checks choose hardware from the branch or PR base: one target on a hardware branch, all three projects on dssmm. Historical test baselines require a full Git checkout; CI fetches full history/tags.
- Direct known hardware-branch PRs into another hardware branch or dssmm are rejected. A whole multi-project PR into a single-project branch is also rejected. Adapt intended fixes on a feature branch based on the correct destination, within its matching project.
- Policy tests reject mixed identities, foreign root/project directories, misplaced root hardware tools and cross-project source symlinks. Tests/builds can detect regressions; they do not prove that every future edit or merge will be safe.

[GitHub observations](https://github.com/voluptuary/ezfode-dssmm-kernel/blob/dssmm/docs/branch-protection-observation.json) show all four branches were unprotected at inspection. These CI checks are therefore not mandatory server-side protection against arbitrary direct pushes, skipped/edited workflows or merges made despite failures. Required checks/reviews must be configured through repository settings by an authorized administrator if enforcement is desired. No protection gate was bypassed or unrequested review requirement imposed.

Initial hosted runs failed on container/checkout UID ownership in historical Git fixtures, after successful kernel builds. The corrected workflow trusts only the exact GITHUB_WORKSPACE, never a wildcard. BLUE now keeps real Git history rooted in the checkout while its copied fixtures/headers use only actual BLUE source. The final CI disposition is recorded in HANDOFF/PROGRESS.

## Remaining limitations

Hardware validation remains model-specific. Original OMEGA and BLUE still have no physical qualification; BLUE larger-image updater acceptance remains unverified. Static RAM gaps are not runtime stack peaks. Existing flash polling, power-loss metadata limits, rare NES layouts and large-cheat parsing limits remain in each project's audit documentation. Structural cleanup is not a new kernel release; all published tags/assets remain immutable.
