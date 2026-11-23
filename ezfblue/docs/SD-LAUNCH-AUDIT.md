# BLUE SD launch audit

Reports describe A doing nothing for SD games while NOR games still launch.
The card/filesystem details have not been supplied or physically reproduced.
A concrete matching failure is reproduced in stock mGBA with the actual BLUE
launch menu and path helpers: an alias-free long filename returns before the
ROM menu or any transfer. With an8.3 alias, the same fixture reaches handoff.

FatFs explicitly leaves `FILINFO.altname` empty for exFAT (`ff.c:get_fileinfo`).
The launcher previously required the full component to fit its13-byte alias
output when no alias existed. Ordinary long exFAT names therefore failed
resolution silently. The correction preserves the full name and treats the
short alias as optional. Canonical recent/favourite/Last Played paths prefer
FAT aliases when present and retain full components otherwise.

Names clipped by the100-byte directory display record have no usable alias on
exFAT. Launch now resolves their recorded directory-entry offset, checks the
size/type/prefix and bounds, and copies the complete name to RAM. It does not
select a file by a guessed prefix. Existing record sizes and PSRAM allocations
are unchanged. Resolution failures now show the ordinary file error.

`tests/test-blue-launch-paths.py` executes extracted production helpers under
sanitizers: FAT aliases, alias-free nested paths,255-byte names, ambiguity and
read/close failures. `scripts/test-native-launch-mgba.sh` compiles the actual
SD launch UI, resolver and metadata code. It exercises A, quick clean and the
preferred-addon fallback with FAT/exFAT-style entries and a clipped long name.
Its SRAM trace identifies directory resolution, header read, save preparation,
FAT preparation, transfer, patch and page0x400 handoff. The disabled-addon quick
path correctly skips patching.

The native fixture explicitly replaces save writes/history and hardware FAT
transfer/patch/boot boundaries. It does not emulate card SD/NOR protocols or
qualify physical flash/save timings. Source generation renames five side-effect
function definitions; SD_list_MENU and the resolver remain unchanged. Linker
wrappers intercept external storage/RTC/card operations. Test ROMs are not
installable firmware; no private BIOS or emulator modifications are used.

The driver SD transfer still waits for the native controller responses; NOR
uses its parameter-only path. No timing limit or hardware command was changed
without physical evidence. This alias bug can explain matching exFAT reports,
not necessarily every reported launch failure. BLUE remains hardware-untested.
