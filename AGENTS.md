# DSSMM repository isolation instructions

The repository is voluptuary/ezfode-dssmm-kernel. GitHub default is `dssmm`;
retired `ezfode` and `main` branches must not be recreated.

## Project boundaries

- `targets/ezfomegade`: EZ-FLASH OMEGA Definitive Edition Rev. A/B.
- `targets/ezfomega`: Original EZ-FLASH OMEGA.
- `targets/ezfblue`: newer Definitive Edition B / BLUE.

Each directory is a complete independent project with its own AGENTS.md,
HANDOFF.md, PROGRESS.md, sources, assets, configuration, build tools and tests.
Read that project's instructions/journals before editing it. Root contains only
universal orchestration, environment, licensing/provenance and repository logs.
Never move hardware-specific source, assets or settings into the root.

The standalone hardware branches use their own root layouts. `projects.json`
records imported source commits; it does not authorize automatic synchronization.
Never merge a whole hardware branch into another hardware branch or into this
differently structured canonical branch. Adapt/import intended changes only
inside the matching project, preserving its hardware protocols and credits.
Do not symlink target sources/assets across projects. A common bug requires
separate evidence, edits and validation in each affected project.

## Build and validation

`make` shows help. Select `make ezfomegade`, `make ezfomega` or `make ezfblue`.
Run the matching `make verify-<target>`; `make build-all`/`verify-all` explicitly
select every project. Build artifacts stay inside their owning project.
Run `make check-isolation` for root/identity checks. Root CI is read-only and
rejects direct known cross-platform branch PRs; it never auto-pushes/merges or
publishes a release. Pages deployment is manual and restricted to dssmm.
Required branch protection/review enforcement depends on repository permissions;
CI alone cannot guarantee arbitrary future edits or direct pushes are safe.

Keep all HANDOFF.md/PROGRESS.md logs, including historical variants. Checkpoint
before substantial edits, after milestones and before commits/pushes; record
actual commands, outcomes, commit/provenance SHAs and precise next steps.
Preserve user edits, audit history, source attribution, tags and release assets.
Do not discard unrelated files or force/reset branches for convenience.

DE8.3.2 is published from ezfomegade. Original OMEGA Test3 and BLUE Test2 remain
hardware-untested. Software builds, mocks and static RAM bounds do not qualify
hardware timing, saving, flash success or runtime stack peaks. Never transplant
flash/save/reset protocols or claim polling/power-loss atomicity is solved
without evidence. Use automatic GitHub source archives; no extra source ZIPs.

Compartmentalization is a structural change: validate standalone and relocated
builds, compare binaries to their published baselines, and inspect all target
root instructions and active references before committing/pushing. Follow the
current root HANDOFF.md/PROGRESS.md for work still in progress.
