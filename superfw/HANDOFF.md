## 2026-11-12 — SuperFW project bootstrapped

- Added an isolated `superfw/` project from upstream `davidgfnet/superfw` commit `dabbf4909f85e556f7a12972cc8a7b94fc54ac51`. Preserved upstream source, assets, tests, nested workflows and notices; included GPL-3.0 text and provenance. The project covers upstream `sd`, `lite` and `chis` variants. No SuperFW firmware has yet been built or tested in this repository, and no physical validation is claimed.
- Upstream files were imported and the shared registry/dispatch/CI/isolation setup is now being wired on `dssmm`. Remaining: collect configured CI results for the complete upstream test suite, then create/push the `superfw` lane and verify all five branch refs. Keep all target-specific follow-up inside this directory.
