# DS Style Merkin Mix

DSSMM keeps three independent flashcart kernel projects in one repository. Choose the exact hardware before building or installing; kernel files are not interchangeable.

| Project | Hardware | Release branch |
| --- | --- | --- |
| [ezfomegade](https://github.com/voluptuary/dssmm/blob/dssmm/ezfomegade/README.md) | EZ-FLASH OMEGA Definitive Edition Rev. A/B | `ezfomegade` |
| [ezfomega](https://github.com/voluptuary/dssmm/blob/dssmm/ezfomega/README.md) | Original EZ-FLASH OMEGA | `ezfomega` |
| [ezfblue](https://github.com/voluptuary/dssmm/blob/dssmm/ezfblue/README.md) | Newer Definitive Edition B / BLUE | `ezfblue` |

Every branch tracks the same combined layout, with three independent projects directly at repository root. Each project owns its source, assets, build tools, tests, configuration and documentation. Root files are universal orchestration, environment and repository records. [projects.json](projects.json) is the common project registry. Hardware branches develop within their own directory; normal merges into `dssmm` integrate those changes.

With devkitARM, libgba, Maxmod, a host C compiler, Python3/Unicorn and Node available:

```sh
make ezfomegade
make verify-ezfomegade
make ezfomega
make verify-ezfomega
make ezfblue
make verify-ezfblue
```

`make` alone shows help and never selects hardware implicitly. `make build-all` and `make verify-all` select all projects present in the current working view. Each project also builds independently from its directory; hardware working trees can use sparse checkout to show only their matching directory plus universal root infrastructure, while all projects remain tracked. BLUE keeps its vendor linker/resource subproject internally; never install its raw development GBA as an update.

Changes belong to the appropriate target. Merge hardware branches normally into `dssmm`, then merge `dssmm` back to synchronize the hardware branches. Keep target work and its journals inside the matching directory; make universal root changes through `dssmm`. CI checks hardware PR scope, layout and target identity. See [the shared-tree workflow](docs/isolation.md) for sparse checkout and merge instructions. Same-file changes can still conflict; branch protection controls mandatory checks.

The hardware-specific READMEs contain features, credits, download/install instructions and remaining validation limits. Original OMEGA and BLUE releases remain hardware-untested. Source archives are supplied automatically by GitHub; no separate source ZIP is maintained.
