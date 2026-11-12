# Bundled MiGoomba GB/GBC emulator

DSSMM 8.4.0 uses [MiGoomba v0.1](https://github.com/michyprima/migoomba/tree/4b401e2ac7876aed9f89face867fd43d89ce189d)
by michyprima, based on Jaga's Jagoomba Color, Dwedit's Goomba Color and FluBBa's
Goomba. Source commit: `4b401e2ac7876aed9f89face867fd43d89ce189d`.
The pinned upstream source is unmodified. Payload metadata/digests and compiler
identity are in [migoomba.json](migoomba.json). DSSMM 8.4.0 embeds this payload; earlier releases retain their previous emulator.

## Hardware variant and saves

This target embeds **ezo**, 81,876 bytes. The upstream ezo build avoids DE rumble register writes. Upstream intentionally does not write emulator settings on ordinary menu exit in this variant.
Both .gb and .gbc use this payload; the GB ROM starts immediately after its
word-aligned end. The payload fits the kernel's128KiB shared cache, with compiler
and runtime guards before copying. The loader's native save/flash/reset protocols
are unchanged. Existing per-game `SYSTEM/SAVER/<game>.esv` files and64KiB kernel
save handling remain in use.

**Back up existing .esv files before testing.** A generated MBC5/32KiB cart-RAM
battery save written by the former bundled Jagoomba was loaded successfully,
modified and retained across simulated power cycles by this payload. This does
not qualify every game or old save state. Prefer in-game battery saves when
transitioning; compatibility of old emulator save states is not established.
No existing user saves are converted or deleted automatically.

The upstream30 variant save/state/menu cases and12 plain-save conversion cases
all report OK across the three builds. This target's detailed output is in
[migoomba-tests.json](migoomba-tests.json). No new DSSMM/MiGoomba integration has
been tested on physical hardware. Existing platform qualification limits remain.

## Reproduce and validate

Clone the upstream source and check out the exact commit above, then run
`make VARIANT=ezo` with the recorded devkitARM toolchain. To regenerate:

```sh
python3 scripts/embed-migoomba.py /path/to/migoomba/migoomba_ezo.gba
python3 scripts/embed-migoomba.py --check
python3 tests/test-migoomba-payload.py
```

The generator rejects changed hashes, invalid GBA headers, unaligned sizes and
images exceeding128KiB. The focused sanitizer fixture executes the actual GB/GBC
switch cases, checks DMA alignment/canaries and exact append offset, and rejects
zero/oversized sizes. It is included in the normal project verifier.

For optional upstream mGBA validation, follow its tools/gbtest README to compile
libmgba/harness and fetch the public test-ROM collection. On this Linux host the
harness also needed zlib1g-dev and `-D_GNU_SOURCE` with strict C11; emulator code
was not changed. Run savetest.py/goombasave_test.py for this variant and check
the printed result flags (their CLI does not return failure for printed FAIL).
Reproduce the old-container test using:

```sh
python3 tests/test-migoomba-save-compat.py --upstream /path/to/migoomba
```

This uses generated homebrew test ROMs and temporary saves, never commercial
games or the user's save files. Release build artifacts are staged under the ignored build-migoomba-test directory.
See migoomba-release-validation.json and the versioned release notes for current evidence.

## Attribution

MiGoomba: **michyprima**. Emulator foundations: **Jaga / EvilJagaGenius**,
**Dwedit** and **FluBBa**. Upstream acknowledgments also credit **Minucce**,
**Sterophonick**, **EZ-FLASH**, **Nuvie**, **Radimerry**, **Therealteamplayer**
and **Endrift / mGBA** for code, support and testing tools. See the pinned upstream
README/source for full notices; this integration does not relicense the emulator.
