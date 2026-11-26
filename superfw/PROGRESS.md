# superfw progress (compact)

Purpose: current working record, newest first. The full pre-compaction journal is
preserved verbatim at `docs/history/journal-pre-compaction-2026-11-26/PROGRESS.md`.

Keep new entries short: what changed, how it was verified, the exact source
commit/build, and the evidence file. Do not paste full logs or release bodies.

## Latest checkpoint

Build 822 (`superfw-build822`, source `2f73b53b`) is the public release and current Chis hardware-test image. Build 827 (`db4e640`) is the newer local software checkpoint and is not published.

## Recent entries

## 2026-11-26 — Build822 normal publication

Rebuilt sd/lite/chis from 2f73b53b and confirmed exact build822 hashes. Published
superfw-build822 with all three firmware images and folder-art.zip; downloaded
assets match. Notes use the Build769 cumulative catalog plus 822 cartridge icons,
DE-aligned settings rows and the PCE return fix. --latest=false again made 822
latest; DE v8.4.1 was restored by REST and verified. No new hardware claim:
Chis822 owner reports cover boot, Atari plugin load/Exit and PCE Exit only.

## Recovery, owner822 report and external Atari migration

Recovered clean827/db4e640 all3 builds and passing sourceCI45402219616/all4
integrationCI45402235990. Local full verifier stopped on the new workspace test's
fixed5s marker; retained that failure, reran focused cases successfully, completed
remaining Start/plugin/MiGoomba/resource/host checks, and added bounded completion
polling with a distinct pending color. Bothfonts TitleH/BoxV simultaneous real
save/reload/art checks pass. Kernel inputs/827 bytes unchanged by fixture fix;
docs/build-827-validation.json records composed evidence and limits.

Owner reports Chis822 boots and Atari games load/Exit successfully; exact plugin
revision unspecified. Stop Atari development here: active repository is now
https://github.com/voluptuary/a26.gba and is already ahead. Old recipes/journals
are historical. Original owner a26.zip upload preserved exactly in project
history/imports (SHA d1f97f8b...), removing the need for a root binary that fails
isolation. No Atari engine changes made during this recovery.

Read-only harness review at external66017dcb: added optional pinned mGBA-core
SuperCard model, real image-backed Lite SD reads and memory-multiple callbacks.
Chis NOR/non-Lite ports/plugin handoff remain unqualified. Do not substitute it
for the existing native mocks or combine EZ drivers; retain as an optional future
bus-testing extension. Current change improves SDL fixture completion readiness.
Next: finish default-branch integration/sync and verify a fresh clone can build
all kernels, then owner-requested19px SuperCard top bar and matching colors.

Owner follow-up: delete obsolete a26.zip rather than retaining a live archive.
Removed the temporary project-history ZIP; original upload remains in Git history.
Only lightweight provenance metadata remains. Atari development stays external.

## 2026-11-24 — Shared workspace candidate

Before adding more settings pages, recover main EWRAM by sharing the9600B selected
art hot copy and8192B settings snapshot in one aligned GBA-RAM union. Explicit
settings lease blocks hot-cache writes until parsing/commit/readback/rollback ends;
generation invalidates stale copies afterward. Five SDRAM artwork slots,8192B
config capacity, original transaction semantics and minimum16KiB stack remain.
No byte-parsed settings on cartridge SDRAM. New module is menu_workspace.c/.h.

Focused host/FAT16/FAT32 all3 settings +cache/generation tests, ARM cart-bus tests,
actual linked ARM save/reload and native mGBA simultaneous save/art in TitleH/BoxV
with bothfonts pass. Development Chis link EWRAM248504 (8520free), IWRAM15256;
recovered8144B; native stackpeak14152B unchanged. This is not yet a clean numbered
hardware-test replacement: keep822 until full all3/native/mGBA/IGM gates pass.
Freeze this source and run complete gates next. PR29 (head4fcc015) independently
covers prior Atari guards and822 evidence; merge it only after checks pass, and
do not change HEAD during the next three-board build sequence. Do not combine
new View/submenu features before this memory/ownership checkpoint is validated.

## 2026-11-24 — Build822 validated and delivered

Full local all3/nativeARM/host/mGBA/nativeIGM verifier exits0, including42 menu
visual checks (Settings/Games/Hardware additions),8 NOR carousel checks, native
save/reload and real plugin Exit/MiGoomba fixtures. Chis822/source2f73b53 is in
hardware-test; old799 removed only after matching its recorded hash. All3 clean
builds share source-tree8932bc31...; sizesSD497152/Lite496640/Chis1671680B.
Chis EWRAM256648/257024 (376B free), IWRAM15240; minimum16KiB stack retained.
SourceCI45395110531 and all4 PR28CI45395123043 pass; PR28 merged37d31fa. Report
is docs/build-822-validation.json. No new physical qualification/public release;
DEv8.4.1 latest verified. Atari plugin14d28e63 remains separate, with source ZIP.

Root dssmm and EZ lanes synchronized normally to37d31fa; SuperFW8e54546 includes
that integration plus the later Atari raster guards. Follow-up hardware PR next.
Do not rebuild822 for tool/doc-only changes. Before adding sizeable UI features,
recover main-EWRAM headroom: explore sharing the9600B selected-art cache with the
8192B settings snapshot, with strict ownership/generation guards and full native
save/navigation tests. Do not move byte-parsed settings into cartridge SDRAM or
reduce settings capacity/stack. Current hardware-test artifact must remain until
any future replacement passes the complete gates.

## 2026-11-24 — Atari raster memory guards

Native replay reproduced all three display queues writing81st entries into80-slot
arrays; a real wide-copy player atx160 also writes2B past colvect230. Added queue/
index guards and256B padded collision scratch (all declarations match), with28
queue/index plus3 wide-copy ARM cases. Current external plugin14d28e63ac2d930d8c1849e030a3ff0701e59bc6e940863d5de9884e6b5f4500
passes original input/timer/control, mGBA runtime/menu10 and timer-gated image;
exact corresponding-source ZIP independently reproduces it. hardware-test plugin/
source/notes/JSON updated. No sound/broad game/physical qualification claims.

Build822/source2f73b53 all3 kernels finished clean with identical source-tree hash
8932bc31... before these unrelated optional Atari-tool edits. Native visual/full
gate remains running; don't rebuild firmware for the external-tool change. CI PR28
head stays2f73b53 until its check/merge checkpoint; commit Atari follow-up locally
and push after that merge to avoid restarting the same CI. Preserve prototype
and current kernel/plugin artifacts, private BIOS/session scratch/ownerlock.

## 2026-11-24 — Settings visual parity candidate

Compared native DE Settings/Games captures against linked Chis799. Add native
scenes20/21/22 for Settings home/Games/Hardware via actual category input, without
requiring NOR. Correct DSSMM-font text baseline and13px selection fill/14px row
against the common DE Interface-label pixel mask. Native font spacing remains.
Host highlight bounds pass; full clean all3/native/mGBA gates must pass before
replacing hardware-test Chis799. Atari current plugin/source remain delivered
separately; public769 unchanged. Continue without waiting for hardware feedback.

## 2026-11-24 — Atari timer/WSYNC corrections

The unfinished core read an uninitialized timer value and subtracted time during
WSYNC. Replaced countdown/underflow/status logic, advance clocks in all beam
states, keep palette declarations256 entries, and poll menu every64 emulated
instructions instead of each instruction (still works without video sync).
Actual ARM passes64 timer boundaries including32-bit wrap,20 WSYNC cases and20
real TIA/RIOT input-port cases; original21 input/12 bank/7 return checks retain
passes. Actual runtime mGBA exact picture/menu10 passes; a new original6502 ROM
waits for RIOT underflow before drawing: olde5cdd3ed fails, corrected1b344401 passes.
No private BIOS/card emulation. Counted keypad reads fall5000→79 per5000 emulated
instructions; this is not an FPS/hardware speed claim. Extra correct WSYNC work
increases total cost relative to the broken baseline; no net speed claim.

Updated hardware-test/SYSTEM/PLUG/a26.gba SHA1b34440155cbca8719ecd4329440230bf0effcabe3e3aa91a7f2d62b46b7e69a
and exact independently rebuilt corresponding-source ZIP/notes/JSON. No sound,
physical Exit, broad games or cycle-exact emulation qualification; Chis799 and
public769 remain unchanged. Resume menu parity comparisons after committing this
checkpoint; no user input needed. Preserve current plugin+source in cleanup.

## 2026-11-24 — Experimental Atari hardware-test plugin delivered

Owner explicitly requested the plugin in hardware-test. Delivered
SYSTEM/PLUG/a26.gba (SHAe5cdd3ed708fb6ad59f205299878fbff1ef13998c745032b4e19df0c77d5ca7b),
setup/limits text, validation JSON and exact prepared-source ZIP with original
notices/CREDITS, GPLv3 text and standalone rebuild script. Independently extracting
and rebuilding the source ZIP gives the identical binary. Runtime2K/4K/F8/F6,
L+R menu, no sound; only generated diagnostics qualified. Actual runtime image
with fixed-ABI cold-boot seeding passes exact pixels and ten mGBA menu/Exit-marker
checks, plus21 input/12 bank-hotspot/7 return-header ARM checks. No public release
or firmware change, no physical-card or broad game/Exit/speed qualification.

Current local kernel remains Chis799. Generic extension loader already supports
this external asset. Preserve it alongside its source/notes during cleanup. Owner
will test when convenient; continue Atari compatibility/performance and DSSMM
porting autonomously. Local source archive is not a GitHub firmware source asset;
do not upload it through release tooling. Tracked packaging/runtime replay tools
make this environment reproducible without private BIOS or old prebuilt objects.

## 2026-11-24 — Atari classic menu prototype

Implemented owner-requested L+R/D-pad/A/B classic emulator menus in the separate
source recipe: Controller, Display, Other Settings, Help, Resume, Restart, Exit.
L alone no longer resets. mGBA exposed stale menu pixels on Restart; restoring
the saved framebuffer fixes it. Ten real SDLmGBA0.10.1/Xvfb interaction checks
pass, including Exit to a deliberately substituted return marker (not physical
firmware remapping). No BIOS or emulator changes. Actual ARM startup +21 loader
cases,12 F8/F6 bank hotspots and7 invalid/valid return-header cases pass. Original
6502 exact-pixel diagnostic passes. Native evidence is in atari2600-prototype-
validation.json; replay tools are tracked under tools/.

Runtime remains a source-only experiment: no sound, only the generated diagnostic
qualified, other Atari bank formats unsupported, upstream COPYING/provenance
review and physical compatibility/Exit remain. No plugin distributed; Chis799
hardware-test and public769 stay unchanged. Continue Atari source/license/core
compatibility review, then supply an explicitly experimental external a26 plugin;
subsequent menu parity pass should fix the native Info unlock-hint clipping.

## 2026-11-24 — Clean777 validated and delivered

Full clean verify-superfw passed all3/native ARM/host/mGBA (including both PCE
Exit paths); native IGM exact ELF/halfword replay passed. Source45291139008 and
all4 PR22 CI45291148001 pass; PR22 mergeddeaf85a4 after EZ SD correction releases.
Chis777/source787ed0d4 is in root hardware-test with validation JSON; replaced769
only after old hash matched. No new physical PCE Exit/save/SD-Lite qualification.
Chis mainEWRAM256568/257024, IWRAM15240; Lite IWRAM15648;16KiB stack unchanged.
SD/Lite flash headroom27648/11264B. Public769 unchanged. Preserve compact resources
and do not spend remaining456B Chis main-RAM without recovery. See build-777 JSON.

Atari separate /tmp prototype now compiles current devkitARM and renders generated
4KiB solid-color6502 diagnostic in stockmGBA (160x96 at35,25). Source-only recipe
normalizes case/globals, corrects palette256 bound and moves large buffers to
EWRAM; old objects/embedded games excluded. Not an external plugin or safe physical
startup yet: standard gba.specs startup WAITCNT needs review, runtime arguments,
return, timing, controls, banks and licensing/provenance remain. Retain reproducible
recipe in the project before session close; no Atari core bundled/claimed.

Atari source recipe and optional mGBA pixel check now tracked under tools/;
ATARI2600-PORT-PLAN.md documents clean pinned source/build/run and limitations.
Modern GCC16.1 source build and exact diagnostic-frame test passed; validation
JSON records hashes. No Atari binary/core imported, no firmware inputs changed,
no new dependency/BIOS. Continue with safe startup before runtime Pogo arguments,
Exit and controlled timing/controls/bank tests; resolve missing COPYING before
binary distribution. Chis777 remains the validated physical-test candidate;
this optional prototype does not create a replacement firmware build.

2026-11-24 continuation: Atari custom ROM startup preserves fixed96B ABI,
clears owned BSS/VRAM only and establishes safe WAITCNT/bitmap state. SDK whole
EWRAM clear, not an established fast-wait write, was the confirmed startup issue.
Linked ARM startup +19 runtime2/4KiB copy/mirror/bounds cases pass; mGBA exact
6502 diagnostic passes through the same loader. No runtime Atari binary supplied:
Exit/banked formats/audio/controls/performance/licensing still pending.

Owner reports PCE Exit now works; recorded without assuming an explicit test
build/backend. Reworked HuCard portrait silhouette after viewing online photos;
added reference-informed NGP/NGPC and Atari cartridge icons, original pixels,
existing palette/read-only stream, no photo assets. Host all3/resource ARM checks
pass; native full clean build follows. Added opt-in D-pad showcase mode to existing
native NOR fixture, with synthetic landscape artwork and no launch keys, for
owner-requested README GIFs. Default regression scenes remain unchanged.

Clean799/source30ff43a7 full all3/ARM/host/mGBA/nativeIGM passed; sourceCI45303504271
and all4 PR26 CI45303565021 passed. Chis799 delivered in root hardware-test after
checking/removing superseded777 hash. All boards clean, Chis EWRAM256600/257024
(424B free), IWRAM15240;16KiB stack unchanged. No Atari binary included, public769
unchanged. Owner PCEExit report retained without assuming build/backend.
README GIF work follows with opt-in original sample art; do not rebuild firmware
just for fixture/doc changes. Preserve799 as the current physical-test candidate.

README preview continuation: optional --art-dir imports the original fictional
Title/Box PNGs using the production red-fast216-color cube. Showcase disables
intentional magenta palette/back-page poison and enables real Clean list; ordinary
regression modes retain poison. Exact default scene15/font0/Light/gray GBA hash
633d9ee485d6ca727eb2afe9f11a442f085311e636b95a8247ca630ab25df88f stayed byte-identical.
Actual799 horizontal/vertical motion captures and Start/options previews pass
visual inspection. No new firmware needed for these fixture-only changes.

Owner clarified branding: DS Style Merkin's Mix / Merkin's Mix in current docs;
historical journals/releases retain exact records. Owner799 .a26 Unknown type is
expected without an installed plugin; now prioritize a working MikeDX or PocketStella
external a26 plugin with familiar L+R menus, D-pad/A/B and normal Resume/Reset/Exit.
PocketStella pin1e299b9 has disabled/absent core/video/audio; existing MikeDX native
startup/ROM/render tests are the practical starting point. No a26 binary delivered.
Native Info screenshot also exposes an overlong unlock hint at the bottom; fix
its wrapping/alignment in the subsequent menu parity pass, preserving exact words.

## Recovery / DE launch audit takes priority

Build776 stopped cleanly at Lite link: IWRAM16160 exceeded unchanged stack
start0x03003f00 by32B. Nothing776 was delivered. Moved the fully written-per-frame
512B sprite command buffer to GBA EWRAM (full64 capacity kept, hot pixel code
and16KiB stack unchanged). Partial Lite link nowIWRAM15648/EWRAM241944; memory
guard and36 actual ARM odd-edge cases pass. Full clean777+all3/CI remains pending.
Public/hardware-test769 remains frozen. Source776 includes PCE return/icon and
Start fixes with focused tests; no new physical Exit qualification.

Owner now prioritizes DE SD-launch regression versus Frankie8.2 before other
porting. No interrupted process remains. Continue that audit in ezfomegade lane;
then resume clean SuperFW gates and Atari source prototype. Preserve ownerlock,
private BIOS, artwork archive and session scratch.

## Owner769 PCE follow-up / next candidate in progress

Owner confirms PCE games load at769, but Exit restarts the emulated game and
PCE has no cartridge icon. Recorded prime-build769-hardware-observation.json;
updated current769 known-issue note after archiving its prior body. DE latest
restored/verified v8.4.0; public769 assets unchanged. Backend is not established
by the report. Native PCE lacked portable's known Exit adaptation; candidate
now applies that adapter only to PCE emulator bytes with a validated return
image. Actual native loader ARM and native ROM/header mGBA Exit pass; hardware
return/save timing remains unqualified. GB/GBC bytes remain unchanged.

Added HuCard-style PCE icon using existing shared palette. Unified cartridge
generator/tests replace GG-only names. ICN1 includes both extra tiles; moving
GG out of EWRAM recovers280bytes of data/upload code. Icon pixel/bounds/route
tests pass all3, actual read-only ARM resource oracle passes. No owner ZIP edit.

Start Box geometry now37x37 at39,48 (Title56x37 at30,48). Odd-edge compositor
passes host oracle and36 actual ARM cases (halfword VRAM,332B function stack);
all4 native Start art/style/font captures pass. Full clean all3/CI is next after
focused review; hardware-test/public769 remain frozen. Partial link before
loop-bound experiment used256056/257024 EWRAM and15752 IWRAM,16KiB stack kept.

Atari2600 is now an additional owner goal. Current PLUG pack has no a26 core;
found MikeDX/2600_gba pinned2eb18dd8, legacy Virtual VCS/X2600 GPL source.
See ATARI2600-PORT-PLAN.md. Prototype stays outside tracked source until it
builds and renders generated homebrew; do not ship missing embedded ROMs or
old prebuilt objects. Continue this investigation during clean current-fix gates.

Rejected pair_end scaling-loop variant:93,679 vs93,597 instructions and336 vs
332B stack for the same linked120x80 opaque case; restored the prior form.
These counts are not physical timing. Current focused36-case compositor,
Start art/font4cases, native PCE loader+Exit mGBA, shared icon resource oracle
and all3 icon checks pass. Final partial link256056/257024 EWRAM,15752 IWRAM.
Next: clean commit/build all3 and integration before replacing hardware-test769.
