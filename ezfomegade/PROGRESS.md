# ezfomegade progress (compact)

Purpose: current working record, newest first. The full pre-compaction journal is
preserved verbatim at `docs/history/journal-pre-compaction-2026-11-26/PROGRESS.md`.

Keep new entries short: what changed, how it was verified, the exact source
commit/build, and the evidence file. Do not paste full logs or release bodies.

## Latest checkpoint

DE 8.4.1 (`v8.4.1`, source `a948b936`) is stable/latest; the optional-alias and clipped alias-free SD-launch regression is software-fixed and released. Affected-card physical confirmation is still pending.

## Recent entries

## 2026-11-27 — Recent artwork path correction

DE source afb649f retains complete names beyond the old 99-byte index limit and
loads Start/Recent artwork using the resolved ROM directory. The project
verifier passed. An exFAT/mGBA card-model fixture showed a visible Recent name
with generic art before the correction and matching art afterward. See
`docs/recent-art-2026-11-27.json`. Owner hardware qualification remains open.

## 2026-11-23 — DE SD-launch regression investigation

Owner prioritizes this before SuperFW porting. Frankie8.2 referencecff4b2f
copies the record filename; DSSMM resolver rejects long alias-free names before
opening the launch menu. exFAT altname is empty by design. Actual DE native
mGBA reproduces before d701ee (directory only), FAT control d7ffaa. Corrected
optional aliases, canonical paths and bounded offset-based clipped-name lookup
pass all5 native cases and sanitized host paths. No driver/save/transfer/fragment
guards changed. Full DE verifier running next; no physical confirmation/release.
See SD-LAUNCH-AUDIT.md and sd-launch-validation.json. BLUE already has this
pure-path correction; OMEGA clipped-name audit and SuperFW gates follow.

Full DE verifier passed (43 host scripts, native five-case mGBA launch, production build, browser and memory checks). Removed duplicate native invocation; host-suite discovery retains it. Owner authorizes corrective release8.4.1; physical affected-card confirmation pending. OMEGA/BLUE audit/publication follows.

2026-11-24: v8.4.1 published at a948b936 after source45293394157 and all4 PR23 CI45293418303 passed; PR23 merged70f7230. Firmware and full folder-art.zip downloaded byte-identically; canonical notes match, GitHub latest explicitly verifiedv8.4.1. SD regression software-reproduced/fixed; affected physical-card confirmation pending. Publication JSON retained. Synchronize via dssmm; no other driver changed.

2026-11-24: Added real carousel-motion input and optional original title/box
artwork to native preview for owner-requested README GIFs. All9 native mGBA
scenes pass, production code unchanged. L/R scene navigation remains; no game/
flash launch path. Root capture tool/media remain universal on dssmm.

## 2026-11-07 17:31 UTC — 8.3.0 follow-up release and repository cleanup started

- **User request:** incorporate the hardware-tested F-011/F-036 fixes into this repository's existing 8.3.0 release, update release notes, commit/push/create a PR, rename the audit ledgers to uppercase filenames, update every current reference, and remove superseded `files/carousel-performance/` and `files/upstream-ui/` artifacts if no active workflow needs them.
- **Target clarification:** the PR/release target is this checkout's `origin`, `voluptuary/ezfode-dssmm-kernel`. `merkin-agent/ezfode-dssmm-kernel` is the fork where this work was previously carried out and is not the current publication target. The user clarified this after an initial mistaken interpretation; no Merkin release or branch was modified.
- **Starting state:** branch `fix/save-and-settings-transactions` at `fc430da` (current `origin/main`), with F-011/F-036 source/test work and audit notes uncommitted. Existing `origin` release `v8.3.0` has the earlier 2,012,736-byte asset. Existing `CHANGELOG.md` already carries the cumulative release sections.
- **Folder decision:** verified `files/upstream-ui/` and `files/carousel-performance/` are referenced only from historical audit prose and their own raw logs; no production source, test script, or build uses them. `test-upstream-ui-host.sh` compares current code to Git commit `1cca2d6` and does not load those folders. Their integration/results are summarized in `files/REVIEW_ARCHIVE.md`, `HANDOFF.md`, and this ledger, so the raw folders are being removed and obsolete path claims are being corrected.
- **Changes already in progress:** renamed `handoff.md`/`progress.md` to `HANDOFF.md`/`PROGRESS.md`, replaced current references to lower-case names and the former prompt filename, updated the audit script's checkpoint filename, added the save/settings fix to the 8.3.0 changelog, and removed the superseded evidence folders and user-authorized integrated Claude patch archive. Root `AGENTS.md` is the current instruction file. A duplicate `files/AGENTS.md` deletion was already present in the shared worktree; preserve that state.
- **Next:** correct historical status wording and stale references, refresh the official candidate at `files/candidate/8.3.0/`, run the full audit/build, prepare the source ZIP from the final commit, commit and push to `origin`, create and merge the PR if repository rules permit, then update this repository's existing v8.3.0 tag/release assets and verify remote hashes. Never publish the candidate to `merkin-agent`.
- Completion: all this platform's published release bodies match canonical notes on GitHub; release metadata/assets are unchanged. Shared commit72f5fe293f8a3ec62ddf24852917af0b340e9b4f is synchronized across all branches. No kernel inputs changed.

2026-11-21: Added opt-in own-kernel mGBA native visual reference with nine
separate-boot scenes, deterministic clock/files/artwork and SRAM completion
checks. No hardware action/driver change. See docs/NATIVE-MENU-PREVIEW.md.
Production verifier and screenshot validation recorded before integration.

Native9-scene stock mGBA run and normal full project verifier passed. Added
Start clock draw used by the real native Start loop and retested scene0. Exact
capture/source hashes and limits: docs/native-menu-preview-validation.json.
No production behavior/driver changes or hardware qualification.

## 2026-11-07 17:15 UTC — F-022/F-032 evidence recheck after hardware report

- **Objective:** Test whether remaining NOR issues have a revision-agnostic code fix using current repository mapping and primary vendor/product sources.
- **Repository evidence:** `source/ez_define.h` maps settings, save records, and NOR catalog to `0x7B0000`, `0x790000`, and `0x7A0000`; `Save_info` erases a full `0x10000`-byte sector. No project-owned revision-specific reservation map documents `0x7C0000`. It cannot safely be treated as spare merely because current kernel metadata uses three other offsets.
- **Primary-source check:** EZ-FLASH's [original Omega DE download page](https://www.ezflashomega.com/pages/EZ-Flash-Omega-Definitive-Edition-Downloads.html) says the latest package is applicable to Rev.A/Rev.B, that Rev.B firmware added S98 `020` support, and that an earlier firmware fixed EEPROM-save behavior on a “specific S71 chip.” It does not identify that S71 orderable part or define a spare S71 sector map. Infineon's [S29WS512P datasheet](https://www.infineon.com/assets/row/public/documents/10/49/infineon-s29ws512p-s29ws256p-s29ws128p-512-256-128-mb-32-16-8-m-x-16-bit-1.8-v-simultaneous-read-write-flash-datasheet-en.pdf) confirms the sector-erase table's 3-second maximum excludes the embedded `00h` preprogram step. Those values therefore do not bound total erase duration. No source establishes identical S71 parts/protocol/timing across all accepted revisions.
- **Conclusion:** No hardware-agnostic F-022 timeout or F-032 shadow sector can be added safely from the available evidence. The missing inputs are actual S71 part markings/revision-to-part mapping and an authoritative firmware/kernel S71 allocation map. Keep both findings open; no NOR code changed. Third-party board reports were not used as allocation authority.
- **Validation/state:** Read-only source/vendor review only. The current candidate remains 8.3.0, and the user's normal boot/save/settings hardware result is recorded above. No commit, push, or release change.

## 2026-11-07 17:13 UTC — Owner hardware report for save/settings candidate

- **Phase/subsystem:** F-011/F-036 candidate validation. **Candidate (now at release path):** `files/candidate/8.3.0/ezkernelnew.bin`, SHA-256 `bf5e34f5d26d6bad846fbce67704414238f38f3d90f68d3196133f780264a92f`.
- **User-reported hardware result:** “this build loads fine, saves seem to work, settings are retained across power cycles.” This is direct owner hardware feedback. It confirms normal boot, ordinary save use, and persistent settings on this image; save-mode specifics and fault/power-loss injection were not reported.
- **Updated assessment:** The earlier startup hang was not reproduced on this candidate, and the settings transaction passed both host fault injection and normal hardware persistence. Keep the startup root cause unknown; do not claim it was diagnosed. F-036 ordinary write-failure preservation is host-injected and normal save operation is hardware-reported; FatFs rename interruption remains an untested recovery limitation. Do not call SD power-loss recovery hardware-verified.
- **Remaining review:** F-022 still lacks complete timing limits and verified IDs for all supported S71 parts; F-032 still lacks a revision-safe spare catalog sector. No code change to either is justified from the evidence currently in the repository. The candidate contains no F-022/F-032 changes.
- **Repository state:** The local branch was aligned to `origin/main` at `fc430da` after the AGENTS rename/update. Firmware and audit-log work remains uncommitted and unpushed; v8.3.0 release assets remain unchanged.
- **Next:** Finish an offline review for hardware-agnostic F-022/F-032 mitigations using repository maps and primary vendor specifications. If that confirms the existing evidence gap, record them as blocked on exact cartridge/flash-part data rather than adding guessed protocol changes. Preserve the user report and candidate hash in handoff.

## 2026-11-07 17:03 UTC — SD save/settings transactional replacement work

- **Objective:** Address the verified F-036 existing-save loss and F-011 settings truncation/write-error risks without changing NOR protocols or adding unmeasured UI/render work.
- **Implementation:** Added `source/file_transaction.h`, a shared staged writer for `/SYSTEM/SETTINGS.TXT` and single-basename files under `/SYSTEM/SAVER/`. It stages and closes the full payload before touching the old destination, checks write/close errors, journals the absolute path and whether a prior destination existed, renames the old file to a recoverable backup, promotes the stage, and runs recovery before settings reads, ordinary save loads, and manual FRAM save operations. Transaction work buffers are placed in EWRAM so the target doesn't consume additional scarce IWRAM.
- **F-036 regression:** Changed `Save_savefile` and `SavefileWrite` to use staged transactions. The host harness now sweeps every `disk_write` call in first-save creation, verifies failed existing-save replacement preserves the old LFN save byte-for-byte, covers full media, successful replacements, settings replacement, and recovery states.
- **FAT rename boundary:** Repository FatFs registers the destination entry before deleting the source; an interruption can leave duplicate directory entries that may reference the same cluster chain. Recovery therefore refuses to unlink or guess in the ambiguous duplicate-entry states, preserving all transaction files for manual recovery. This avoids automatic destruction but can block further save/settings access after power loss during the rename critical section. This residual limitation is explicit; the host test models the ambiguous state. No physical SD-card interruption test was available.
- **NOR triage:** Rechecked `Wait_Flash_Operation`, `Save_info`, `NOR_EraseAllPPB`, and `Save_info_if_changed` against the existing F-022 evidence. A finite timeout still cannot be justified across accepted/unknown S71 IDs or the unbounded datasheet erase preprogram phase; changing polling or adding time constants would be a hardware-specific guess. F-032 still has no verified spare 64 KiB S71 sector across supported revisions; the prior `0x7C0000` shadow proposal remains rejected. No NOR code changed.
- **Validation:** Forced production build passed: `omega-de-ds-style-multisystem-kernel.gba`, 2,014,248 bytes (82,904 bytes below 2 MiB). Focused `bash scripts/test-savefile-host.sh` passed, including injected write failures, byte-for-byte old-save preservation, settings persistence, and safe refusal on ambiguous FAT rename states. `scripts/rebuild-and-verify-audit.sh` completed successfully: all 31 host scripts, browser scenarios, JavaScript syntax, whitespace checks, and configured Cppcheck on four firmware files passed. No hardware test has been performed.
- **Candidate:** `files/candidate/8.3.0/ezkernelnew.bin`, 2,014,248 bytes, SHA-256 `bf5e34f5d26d6bad846fbce67704414238f38f3d90f68d3196133f780264a92f`. Experimental only; About version remains 8.3.0. Candidate README calls out the unresolved power-loss rename window and requests checks for startup, artwork/menu responsiveness, settings, GBA save export/load, and manual FRAM save.
- **Remaining:** F-011's write-error truncation is addressed in host evidence, but the earlier hardware startup/thumbnail stall is still unexplained and this new integrated image needs hardware testing before the finding can close. F-036's ordinary write-failure data loss is addressed; power loss during FatFs directory rename remains an explicit recovery limitation. F-022 and F-032 are not safely solvable from verified evidence in this workspace. Do not publish this candidate as a release.

## 2026-11-02 — Archived final parallel-audit patch 0018

- Added patch 0018 as `claude/patches/0018-Screen-artwork-rows-for-the-transparent-sentinel-two.patch` in the then-tracked Claude archive, completing the parallel audit's patch sequence; the archive was later retired after integration. Confirmed all 18 archived patches apply in order to base `83e350d`.
- Updated the archive README and Claude handoff/progress records. They distinguish the preserved submitted proposal from the adapted, shipped v8.1.0 implementation: the latter checks exact sentinel candidate pairs during the batched scan instead of rescanning transparent rows.
- Owner hardware testing of the integrated v8.1.0 build reports responsiveness about the same as the prior build, with a possible marginal improvement; the host differential/fuzz and target-build evidence is recorded above.
- Current archive: 344,979 bytes, SHA-256 `118d8e4875e6b7434bdb50bdda9c25b7fb54895cdc88418183c8e68c62a98e1c`. The pre-existing archive members outside the three updated archive records and the newly added patch were verified byte-identical. The duplicate standalone copy has now been removed; the archive retains the exact original patch (SHA-256 `d817ecaff96bf042323d5aaa26121764e9c6de62ce1034d39556cfe2feb3d95c`).

## 2026-11-02 — Published DS Style Merkin Mix 8.1.0

- Bumped the About-screen build label to `DS Style Merkin Mix 8.1.0`.
- Updated the cumulative release notes through v8.0.0, including the newly hardware-tested artwork row-scan optimization. The notes ask users to report responsiveness in the [GBAtemp DS Style thread](https://gbatemp.net/threads/ds-style-a-nintendo-ds-inspired-custom-kernel-for-ez-flash-omega-omega-de.680565/).
- Published stable [v8.1.0](https://github.com/voluptuary/ezfode-dssmm-kernel/releases/tag/v8.1.0) from commit `8bbd5545240fe3d2176f5e63f30a889e0c5d9238`, with annotated tag `v8.1.0`. Asset `files/candidate/8.1.0/ezkernelnew.bin` is 2,066,480 bytes, SHA-256 `a43002724fca4792f21865e2c61eb02e4184d14809efad915dfcd90a8fc93541`; the downloaded release asset was byte-identical. GitHub reports the release as non-draft, non-prerelease, and latest.
- The build, complete audit script, all 13 host suites, browser installer scenarios, JS syntax, whitespace, and targeted Cppcheck passed. Differential drawing tests matched the prior implementation; helper fuzz had zero mismatches. Owner hardware testing reports comparable performance with a possible marginal improvement; invite broader user feedback, not as a guarantee of measurable acceleration.

## 2026-11-02 — Transparent-artwork row-scan optimization candidate

- Reviewed patch 0018 (then preserved in the Claude archive, later retired after integration) against the current v8.0.0 source. The finding is valid: both row-drawing paths first compare pixels individually to detect transparency before deciding between DMA and masked copying. This exact scan is performed even for opaque artwork.
- Applied the optimization with a correctness adjustment: scan aligned pairs using 32-bit loads, screen batches for bit-15 candidates, and compare only candidate pairs exactly as they are found. This preserves the exact `0x8000` sentinel rule (including non-sentinel values with bit 15 set) while avoiding the patch's second full-row scan whenever transparency exists. Unaligned leading pixels and odd tails use 16-bit reads; nonpositive/null spans return safely. No state or call-site behavior was added.
- Added `scripts/test-launcher-artwork-draw-host.sh` and `tests/launcher-artwork-draw-host.c`, and wired the regression into `scripts/rebuild-and-verify-audit.sh`. The harness compares both drawing routines against reference `83e350d`, checks framebuffer hashes and DMA-copy counts over 120,000 scenarios, fuzzes the helper against an exact pixel scan over 766,800 cases, and uses alignment sanitization to catch unsafe wide loads.
- Validation passed: the differential test reports identical output and 2,650,289 DMA row copies; helper fuzz reports zero mismatches; all 13 host suites passed; the full audit build/browser/JS/whitespace/Cppcheck script passed. ARM disassembly confirms aligned word loads in the grouped scan.
- Hardware-test build: `files/candidate/8.0.0-transparent-draw-test/ezkernelnew.bin`, 2,066,480 bytes (30,672 bytes below the 2 MiB limit), SHA-256 `cbc4c13dca76560c6db2523a5ff2c24a2054d7e69d61a2a627cd7b0175651e92`. The owner subsequently tested the optimized build on hardware and reports performance about the same as the prior build, with a possible marginal improvement; no measured benchmark is claimed.

## 2026-11-01 17:33 UTC — Published DS Style Merkin Mix 8.0.0

- Published stable release [`v8.0.0`](https://github.com/voluptuary/ezfode-dssmm-kernel/releases/tag/v8.0.0) from commit `ed0f530a7e43d72e449ac00ddd6807ccb35e43e4`; its Git tag retains the established leading `v`, while the About screen displays `DS Style Merkin Mix 8.0.0`.
- Uploaded `ezkernelnew.bin` (2,066,312 bytes, SHA-256 `f7b258acd3914f816bc69e8aaa49c270562015b9737e0490989c414fe8980cf2`). The downloaded release asset matched the candidate byte-for-byte; GitHub reports the release as non-draft, non-prerelease, and latest.
- Production build and `scripts/rebuild-and-verify-audit.sh` passed, including the FAT-table regression, browser installer scenarios, JavaScript syntax, whitespace, and targeted Cppcheck. The About string was verified in the binary. Fragmented-card behavior has not been tested on physical hardware.
- Settings persistence no longer reproduces per the user's hardware report for v7.4.6b. Continue the audit; F-022, F-032, and F-036 remain open.

## 2026-10-16 00:47 UTC — Long-LFN save backup path defect

- **Objective:** Finish save compatibility review after widening save identity to the full LFN.
- **Verified finding, Medium:** `Backup_savefile` in `source/ezkernelnew.c` uses two `MAX_path_len` (256-byte) arrays for the absolute backup path `/SYSTEM/BACKUP/SAVER/<basename>`. The directory prefix through the final slash is 21 bytes. Its existing `snprintf` check returns for a 234-byte basename or longer, while `Process_savefile` now accepts save basenames through 255 bytes. With backup enabled, primary save loading still proceeds but this optional backup silently does not happen for long LFNs. This path was not found in the earlier check of primary save buffer lengths.
- **Reproduction by source bounds:** `strlen(prefix) == 21`; with basename length 234, the path length is 255 and trips `written >= sizeof(temp_filename)-1`. Maximum 255-byte basename plus prefix and backup digit/NUL needs 278 bytes, so `MAX_path_len+32` (288 bytes) is sufficient.
- **Planned fix/test:** Enlarge both `Backup_savefile` path arrays and use a small shared checked path-builder helper so host tests can cover the maximum legal basename plus backup suffix. Also create/open the resulting nested path on the host FatFs fixture. Then run host fixture, target `make`, one-config Cppcheck, and `git diff --check`.
- **Current memory state before this extra change:** final link uses `.sbss` 226,360 bytes, EWRAM heap margin about 19,392 bytes, IWRAM ends at `0x03007cd8` with 808 bytes to physical end. Backup buffer growth is local stack storage and does not add static EWRAM/IWRAM; maximum local growth is 64 bytes per two arrays (128 bytes total frame).
# 2026-10-16 02:45 UTC — User-authorized untested non-chip release preparation

- **Objective:** Commit and push the completed changes that are independent of NOR chip protocols, then publish the resulting build explicitly as untested.
- **Current state verified:** fork release `v7.4.3` is published at `voluptuary/ezfode-dssmm-kernel`; feature branch is cleanly pushed through `95e7989`, with F-022 edits still uncommitted. User-owned `files/` remains untracked and untouched.
- **Scope decision:** include only F-026 stack/heap relocation and F-009 LFN/save-path changes, already committed and pushed through `70f6040`. Exclude `e2a8d97` Timer1/S71 NOR execution change and all current NOR/metadata-flash source edits because flash interrupt safety remains open. Exclude the audit ledgers and `files/` from the firmware release asset.
- **Release metadata finding:** `source/launcher_version.h` at the selected source commit still says `v7.4.1`, although the current published fork release is v7.4.3. The isolated release branch needs a version bump and changelog entry before the build.
- **Plan:** prepare `v7.4.4-untested` from isolated commit `70f6040`, build the target image, label the GitHub release prerelease and hardware-untested, and verify the pushed commit/tag/release asset. This is not an audit-complete or hardware-tested release.
- **Commands/evidence:** `git status`, `git log`, `git show`, fork-scoped `gh release list/view`, Makefile/build workflow, and source version header inspected. No build, commit, push, tag, or release has occurred yet.
- **Build milestone:** created isolated worktree/branch `release/v7.4.4-untested` at `70f6040`, changed only `source/launcher_version.h` and the top of `CHANGELOG.md`, and ran `make` successfully. Output is `omega-kernel-release-v7.4.4-untested.gba`, 2,094,112 bytes, SHA-256 `00b8ec49270aee11c8b42474f617ff4a4adedb766ab4233dcd77675116f5414d`. Binary scan confirms the `v7.4.4` version text; `git diff --check` passes. This was build-only validation, with no hardware/runtime testing.
- **Commit/publication status:** release metadata remains uncommitted; branch and tag have not been pushed; no GitHub release exists yet. The original feature worktree remains untouched apart from the audit ledger checkpoint. No source/runtime tests were added or run.
- **At that checkpoint, planned next steps were:** commit the two release metadata files, package a byte-identical `ezkernelnew.bin`, verify its digest, push the branch/tag, publish a clearly marked prerelease, and verify remote state. The later 8.0.0 entry below records the completed stable release.
