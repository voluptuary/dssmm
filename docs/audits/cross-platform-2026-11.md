# Cross-platform adversarial audit — November 2026

Status: active; not a completed audit or hardware qualification.

Baseline refs: DE5f700c9, BLUEdb3db27, Original OMEGA20a1d94. Port releases BLUE Test1 and Original OMEGA Test2 were software-validated and are preserved; stable DE v8.3.1 remains latest. Worktrees isolate all target code and metadata.

| Finding | Severity | Evidence / current status |
| --- | --- | --- |
| PATCH-NES-02: unchecked NES indices and stale state | Medium | Reproduced on all three production functions with sanitizers/assertions. Explicit native/staged spans, pointer checks and retained probe state implemented; ten scenarios incl5,000 poisoned-tail iterations and all target builds pass. Full post-fix audit suites still pending |
| PATCH-NOR-01: DE staged patch-copy overrun | Medium | Reproduced six-byte write at128 KiB minus2; DE intersection correction now passes shared canary/bank tests; Omega/BLUE retain equivalent clipping |
| Native patch capacity preflight | Low / defense in depth | Invalid selectors reproduced in all three helpers; target-specific whole-range checks and last-halfword/bank tests pass; frontend producer reachability unverified |
| Immutable NES blob consumes832 bytes of IWRAM | Informational | DE/Omega now const aligned ROM, matching BLUE; bytes/hash unchanged, ROM symbols and RAM margins checked |
| Save-path ASCII/DBCS policy | Medium (port compatibility), Low (DE policy) | Real FatFs reproduces valid CP936 save rejection on ports and ASCII policy failures on DE; corrected scanner/prefix tests and complete fault suite pass on all targets |
| DE full-width cheat sizes | Low / malformed-input identity |4 GiB+4 fixture reproduces truncated hash validation; full-width guards/clamps and cheat tests now pass |
| DE plugin boot/copy/failure/metadata handling | Medium pending per-case proof | Deferred source/host evidence from porting; actual linked boot and filesystem verification required |
| F-022 raw flash polling; F-032 catalog interruption | Existing unresolved limits | Chip timing/safe sector-allocation evidence absent. Do not transplant another target's protocol or claim host tests solve physical behavior |

Commands: per-target verify scripts for Omega/BLUE; DE production build + scripts/run-host-test-suite.sh, browser/JS checks and configured Cppcheck. Shared NES regression is scripts/test-nes-patch-host.sh. See branch HANDOFF/PROGRESS for exact milestone results, current code state and continuation.

Coverage limits: valid NES signatures are synthetic conformance fixtures, not game execution; bus/BIOS/flash tests are simulated. Uncommon legacy layouts whose probe occurs after its patch output block remain unqualified; no unseen future data is read or claimed to be patched retroactively. No physical tests for either port.
