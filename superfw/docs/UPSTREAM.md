# Upstream provenance

This directory contains SuperFW from [davidgfnet/superfw](https://github.com/davidgfnet/superfw), pinned at commit [`dabbf4909f85e556f7a12972cc8a7b94fc54ac51`](https://github.com/davidgfnet/superfw/commit/dabbf4909f85e556f7a12972cc8a7b94fc54ac51) (`master`, 2026-11-08). The upstream source and its original copyright notices are retained. `COPYING` supplies the GNU GPL version 3 text referenced by the source headers; components keep their own notices and licenses as documented upstream.

`README.md`, the upstream `Makefile`, source, assets, linker scripts, tools, tests and nested workflow files are imported without changes. DSSMM-specific additions are `AGENTS.md`, `HANDOFF.md`, `PROGRESS.md`, `project.json`, this provenance record, `.gitignore` build-output entries, and `scripts/`. The project is built and maintained independently from the OMEGA-family kernels. No OMEGA driver or shared hardware code is used.

To review a future upstream update, compare the pinned commit with the upstream default branch, review its source, assets, third-party licenses and build/test changes, then import the reviewed snapshot into this directory. Do not replace the pin with an unreviewed moving branch or introduce symlinks to another project.
