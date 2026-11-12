# SuperFW project instructions

This project targets SuperCard GBA flash carts and derivatives/clones. Its codebase is upstream SuperFW by davidgfnet, pinned and documented in `docs/UPSTREAM.md`. Read root `AGENTS.md` and this project's `HANDOFF.md` and `PROGRESS.md` before project work.

On branch `superfw`, edit only `superfw/`, including its journals. All repository branches retain the complete shared tree; never remove other project directories. Merge this lane normally into `dssmm`, then synchronize `dssmm` back to this lane. Integrate through `dssmm`, not another hardware branch. Root build/CI/policy changes belong on `dssmm`.

This project has three upstream board variants: `sd` (default SuperCard SD), `lite` (SuperCard Lite), and `chis` (SuperChis). Build one with `make superfw BOARD=sd|lite|chis`; run `make verify-superfw` for all three builds and the upstream host tests. Build outputs stay in `superfw/build/`. Do not assume the variants or their firmware are interchangeable.

Preserve upstream copyright headers, `COPYING`, component notices and attribution. Most firmware source is GPL-3.0-or-later; individual components retain their own licenses. Do not move SuperFW code into another kernel directory or relabel its license. The upstream firmware is intended for SuperCard-family hardware; DSSMM's integration and this repository's builds still require independent validation. Do not claim physical hardware testing without recorded evidence.
