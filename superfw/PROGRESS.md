## 2026-11-12 — Initial SuperFW import

- Pinned upstream default-branch snapshot `dabbf4909f85e556f7a12972cc8a7b94fc54ac51` from `https://github.com/davidgfnet/superfw`. Kept the upstream tree intact and isolated beneath this directory. Read `docs/UPSTREAM.md` for provenance and license notes.
- Setup is in progress. The source has not yet been built or tested in this repository. No physical SuperCard validation is claimed. Next: wire the shared registry, root dispatch, CI and merge/sparse scope policy, then validate each upstream board flavor and host tests before publishing/synchronizing the new hardware branch.
