# Agent quickstart (token-efficient)

This is the compact entry point for agent work in DSSMM. Read `AGENTS.md` for
the charter, then this file. The root `HANDOFF.md`/`PROGRESS.md` and each
project's `HANDOFF.md`/`PROGRESS.md` are **large append-only journals** (the
three EZ-FLASH lanes are roughly 500 KB each and about 90% shared). Never read
them whole; grep their headings and read only the newest entries.

## 1. Read order

1. `AGENTS.md` — mission, ownership, release rules.
2. This file — commands, current state, evidence locations, search hygiene.
3. `HANDOFF.md` §7–§8 only — recent task log and immediate next steps:
   `grep -n '^## ' HANDOFF.md | tail -20`
4. The lane's own `AGENTS.md`.
5. Only the newest journal entries you actually need:
   `grep -n '^## 20' <lane>/PROGRESS.md | tail -20`
   then read that line range.

Skip `docs/history/**` and `*/docs/history/**` unless you are deliberately
investigating history.

## 2. Repository map

| Path | Contents |
| --- | --- |
| `ezfomegade/` | OMEGA Definitive Edition Rev. A/B — stable primary lane |
| `ezfomega/` | Original OMEGA — experimental, hardware-untested |
| `ezfblue/` | OMEGA DE B / BLUE — experimental, hardware-untested |
| `superfw/` | SuperCard SD/Lite/Chis — independent GPL SuperFW fork |
| `docs/` | Current builds, environment, isolation policy, release records |
| `scripts/` | Universal dispatch, policy, packaging and capture tools |
| `tests/` | Isolation/merge/policy fixtures |
| `.github/workflows/` | Read-only CI (never pushes or publishes) |

Root holds only universal material. Develop inside the owning project directory
on its matching branch, then integrate through `dssmm`.

## 3. Build and verify

```sh
make                                  # help
make ezfomegade                       # build one project
make verify-ezfomegade                # build + full project verifier
make ezfomega   / make verify-ezfomega
make ezfblue    / make verify-ezfblue
make superfw BOARD=sd|lite|chis        # SuperFW boards: build sequentially
make verify-superfw
make build-all / make verify-all       # all projects present in this view
make check-isolation
make check-context                     # documentation context-cost report
python3 tests/test-project-isolation.py
python3 tests/test-project-merges.py
```

### Verify/build output is log-first

`make <lane>` and `make verify-<lane>` write the full transcript to
`build/<command>-<lane>[-<board>].log` and print only a verdict plus a bounded
tail (last 60 lines on failure, last 6 on success).

- Read the log only when the verdict is `FAIL`.
- Set `DSSMM_VERBOSE=1` (or run under CI, which sets `CI`) to stream the full
  transcript as before.

## 4. Search hygiene

A root `.ignore` excludes junk from ripgrep/fd-style searches:

- archived journals: `docs/history/**`, `*/docs/history/**`
- packaged manager assets: `*/docs/manager/packages/**`
- generated/vendor blobs: `**/source/fw.h`, `**/source/HZK12.h`,
  `**/source/Newest_FW_ver.h`, `**/ff15/ffunicode.c`, `**/*.pack`, `**/*.hex`,
  `**/*.bin`, `**/*.gba`

Use `rg --no-ignore` (or `rg -uu`) only when you deliberately need the archives.

Do **not** open `source/fw.h` (≈59,500 lines), `HZK12.h`, `Newest_FW_ver.h`,
`ffunicode.c`, or font packs: they are generated/vendor data, not logic, and a
single match can flood context.

## 5. Current state pointers

Verify on GitHub before relying on release state.

| Lane | Checkpoint | Evidence |
| --- | --- | --- |
| DE Rev. A/B | 8.4.1, source `a948b936` | `ezfomegade/docs/` release/native-launch records |
| Original OMEGA | 8.4.1 Test 5, source `29bbb055` | `ezfomega/docs/`; hardware untested |
| BLUE | 8.4.1 Test 5, source `9bc24e04` (reports 8.4.2) | `ezfblue/docs/`; hardware untested |
| SuperFW | Build 822 public, source `2f73b53b` | `superfw/docs/build-822-validation.json`; Build 827 is local/unpublished |

- `docs/current-builds.md` — validated checkpoints and reproduction notes.
- `docs/local-development.md` — container, mGBA/Xvfb, capture tools.
- `docs/isolation.md` — layout, sparse views, merge policy.
- `docs/releases/` and `docs/history/releases/` — canonical notes and archives.
- `HANDOFF.md` §7–§8 — recent milestones and next actions.

## 6. Evidence and journals

- Record results in the owning project's `docs/` JSON/report, not in a giant
  prose entry.
- Journal entries should be short: what changed, how it was verified, the
  exact source commit/build, and a link to the evidence file. Move detail into
  the evidence JSON.
- Never treat a build, mock, emulator run or CI pass as hardware validation.
- Keep the root journals for shared architecture/integration only; routine
  project work updates that project's journals.

## 7. Context budget

Approximate cost of the mandatory reading set (chars ÷ 4):

| Set | Bytes | ~Tokens |
| --- | ---: | ---: |
| Root `AGENTS`+`HANDOFF`+`PROGRESS` | 42,725 | ~10.7k |
| `ezfomegade` trio | 1,301,707 | ~325k |
| `ezfomega` trio | 1,337,736 | ~334k |
| `ezfblue` trio | 1,308,960 | ~327k |
| `superfw` trio | 357,331 | ~89k |

`make check-context` recomputes these numbers and warns when they grow.

## 8. Environment gotchas

- No GBA BIOS, ever. Blank mGBA frames are usually configuration, not a render bug.
- mGBA may be `/usr/games/mgba`; headless needs `SDL_AUDIODRIVER=dummy` and `xvfb-run`.
- SuperFW board variants share intermediates: build them sequentially.
- Build numbers derive from full Git history; use a full clone.
- Lost/interrupted logs are not evidence — rerun the check.
- ASan harnesses may need `-fno-pie -no-pie` (already handled where required).
