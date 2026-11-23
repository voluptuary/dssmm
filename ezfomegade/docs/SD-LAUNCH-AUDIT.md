# DE SD launch regression versus DS Style8.2

Reference: FrankieT19/omega-de-ds-style-kernel commit
cff4b2f4254158944e0a8b182e4b1cf4a22288ab, labelled DS Stylev8.2. User reports
DSSMM8.4 A is ignored for SD games while NOR and other menus work. The reported
card filesystem is unknown; this audit reproduces a concrete matching case.

Frankie8.2 copies the selected record filename before opening its launch menu.
DSSMM first re-resolves it to preserve complete names and canonical identities.
That resolver wrongly required the full name to fit a13-byte short-alias output
when FILINFO.altname was empty. DE's FatFs enables exFAT and explicitly leaves
altname empty for exFAT entries. A long name therefore returned silently before
ROM metadata, save preparation, FAT transfer or launch. NOR bypasses this path.

Stock mGBA, actual DE SD_list_MENU and real path/metadata code reproduce the
failure: FAT alias trace d7ffaa reaches the mocked page0x200 handoff; alias-free
trace d701ee returns after directory resolution without opening the ROM.
The corrected source passes FAT, alias-free exFAT, clipped long filenames,
quick clean and preferred-addon cases. All stages reach handoff; the addon
case with all addon flags disabled correctly skips the patch step.

The correction keeps aliases optional and prefers a full component when no
alias exists, including nested history/favourites/Start paths. A truncated
record resolves by its stored directory-entry offset, with name-prefix, size,
type, close and capacity checks. Sort already preserves those offsets. Failed
resolution shows the normal file error. No hardware command, driver, transfer
timing, save format or fragment guard changes; safety checks added since8.2
remain. BLUE's earlier pure path fix was independently reviewed for DE; its
page0x400 driver/handoff is not imported into DE.

Permanent host tests exercise FAT/exFAT-style aliases, nested and255-byte
components, ambiguity and read/close failures. The native fixture substitutes
filesystem/storage and save/transfer/patch/boot boundaries, not the SD launch
UI. It is not an SD-card emulator, installable firmware or physical flash/save
qualification. No BIOS/commercial ROM or mGBA patch is used.

Original OMEGA already uses full names without requiring a short alias; its
clipped-name path is being audited separately. SuperFW also uses full FatFs
names and has no matching mandatory-alias resolver. Formatting is a plausible
trigger, not proof that every reported user's problem has this cause.
