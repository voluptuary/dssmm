# DS Style Merkin Mix

DSSMM keeps three independent flashcart kernel projects in one repository. Choose the exact hardware before building or installing; kernel files are not interchangeable.

| Project | Hardware | Release branch |
| --- | --- | --- |
| [ezfomegade](targets/ezfomegade/README.md) | EZ-FLASH OMEGA Definitive Edition Rev. A/B | `ezfomegade` |
| [ezfomega](targets/ezfomega/README.md) | Original EZ-FLASH OMEGA | `ezfomega` |
| [ezfblue](targets/ezfblue/README.md) | Newer Definitive Edition B / BLUE | `ezfblue` |

The `dssmm` default branch contains isolated snapshots under `targets/`. Each project owns its source, assets, build tools, tests, configuration and documentation. Root files are universal orchestration, environment and repository records. [projects.json](projects.json) records snapshot provenance. No automatic synchronization or cross-platform merge is configured.

With devkitARM, libgba, Maxmod, a host C compiler, Python3/Unicorn and Node available:

```sh
make ezfomegade
make verify-ezfomegade
make ezfomega
make verify-ezfomega
make ezfblue
make verify-ezfblue
```

`make` alone shows help and never selects hardware implicitly. `make build-all` and `make verify-all` select all projects explicitly. Each project also builds independently from its directory; the dedicated hardware branches retain standalone layouts. BLUE keeps its vendor linker/resource subproject internally; never install its raw development GBA as an update.

Changes belong to the appropriate target. Do not merge a whole hardware branch into another hardware branch or into this differently structured default branch. Adapt/import intended changes in its target directory and run that target's verifier. CI checks layout and target identity and rejects known cross-platform branch PRs; it cannot prove arbitrary future edits regression-free or enforce required reviews without branch protection.

The hardware-specific READMEs contain features, credits, download/install instructions and remaining validation limits. Original OMEGA and BLUE releases remain hardware-untested. Source archives are supplied automatically by GitHub; no separate source ZIP is maintained.
