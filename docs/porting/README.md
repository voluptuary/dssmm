# ezfomega target layout

Hardware identity is recorded in ../../project.json. This branch contains only its target's build/source/assets/tools and applicable tests; canonical dssmm keeps an independent snapshot under targets/ezfomega.

Run `python3 scripts/project.py build ezfomega` and `python3 scripts/project.py verify ezfomega` from the project root. See [OMEGA_STATUS.md](OMEGA_STATUS.md) for origin, hardware limits and historical milestones. No hardware qualification is implied.

DE-only installer documentation, DE hardware tests and obsolete verification wrappers were removed. BLUE additionally removed its excluded DE donor source. Audited removals are in [target-cleanup.json](target-cleanup.json); all HANDOFF/PROGRESS records are preserved. Shared frontend tests and asset generation tools remain. No automatic push, synchronization, cross-target merge or release publication is configured.
