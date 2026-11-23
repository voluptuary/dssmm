# Native GB/GBC emulator

SuperFW Build759 and later use MiGoomba v0.1 by michyprima, based on Jaga's
Jagoomba Color, Dwedit's Goomba Color and FluBBa's Goomba. It is the same pinned
core revision used by DSSMM's EZ-FLASH builds, but the generic hardware variant:
https://github.com/michyprima/migoomba/tree/4b401e2ac7876aed9f89face867fd43d89ce189d
No DE-specific clock, rumble or card driver is imported. Exact payload identity
and compiler/build details are in migoomba.json. Upstream source is unmodified.

SD and Chis replace the earlier bundled Jagoomba; Lite now also bundles the core
where its new resource-compression margin permits it. External PLUG overrides
retain their existing precedence. GB/GBC files append immediately after the
word-aligned native payload. SuperFW's existing SYSTEM/SAVER paths, SRAM save
preparation and reboot backup behavior stay in place. Existing user files are
not converted/deleted; old emulator state compatibility is not established.

The payload/header verifier rejects changed or wrong-variant data. Native ARM
loader tests decode the actual packaged GCEM asset, compare the complete core,
verify GB/GBC append offsets/data and check the native reset boundary. Synthetic
homebrew GB/GBC stripe programs run the exact generic core in stock SDL mGBA;
no commercial games, private BIOS or SuperCard emulation are involved. Keep the
distinction between core smoke tests, loader models and physical save timing.
Build759 clean all-board, ARM, mGBA and integration CI passed; physical
save timing and SD/Lite hardware operation remain unqualified.

The previous Jagoomba binary remains an upstream/reference input, not an extra
firmware asset. MiGoomba foundations/acknowledgments retain their original
credits, including Minucce, Sterophonick, EZ-FLASH, Nuvie, Radimerry and
Therealteamplayer. Refer to pinned upstream source for component notices.
