#!/usr/bin/env python3
"""Build a pinned, emulator-only Virtual VCS diagnostic outside firmware inputs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import shlex
import subprocess
from atari2600.raster_guards import apply_raster_guards

PIN = "2eb18dd8a5b832c939efe510b4e493ad4cf25437"
ROOT = Path(__file__).resolve().parents[1]


def diagnostic_rom(timer=False):
    """Original 6502 code: a solid-color, 262-line frame; no commercial data."""
    code, labels, branches = bytearray(), {}, []

    def emit(*values):
        code.extend(values)

    def lines(count, name):
        emit(0xa2, count)                 # LDX #count
        labels[name] = len(code)
        emit(0x85, 2, 0xca, 0xd0, 0)     # STA WSYNC; DEX; BNE
        branches.append((len(code) - 1, name))

    emit(0x78, 0xd8, 0xa2, 0xff, 0x9a)  # SEI; CLD; LDX #255; TXS
    if timer:
        emit(0xa9, 3, 0x8d, 0x96, 2)    # TIM64T = 3
        labels["timer_wait"] = len(code)
        emit(0xad, 0x84, 2, 0x10, 0)     # LDA INTIM; BPL until underflow
        branches.append((len(code) - 1, "timer_wait"))
    emit(0xa9, 2, 0x85, 1)              # VBLANK on
    frame = len(code)
    emit(0xa9, 2, 0x85, 0)              # VSYNC on
    lines(3, "sync")
    emit(0xa9, 0, 0x85, 0)
    lines(37, "blank")
    emit(0xa9, 0, 0x85, 1)              # VBLANK off
    emit(0xa9, 0x3e, 0x85, 9)           # COLUBK
    lines(192, "picture")
    emit(0xa9, 2, 0x85, 1)
    lines(30, "overscan")
    emit(0x4c, frame & 255, 0xf0)        # JMP frame at $F000
    for offset, name in branches:
        code[offset] = (labels[name] - offset - 1) & 255
    code.extend(bytes([0xea]) * (4096 - len(code)))
    code[-4:] = bytes([0, 0xf0, 0, 0xf0])
    return bytes(code)


def prepare(source, output, fixture):
    # Keep every upstream notice; never consume the old prebuilt object.
    for path in source.iterdir():
        if path.suffix.lower() in (".c", ".h"):
            text = re.sub(r'(#include\s*")([^"]+)(")',
                          lambda m: m[1] + m[2].lower() + m[3], path.read_text()).replace("color[255]", "color[256]")
            (output / path.name.lower()).write_text(text)
    for name in ("CREDITS", "readme.txt"):
        shutil.copy2(source / name, output / name)

    path = output / "gba.h"
    text = path.read_text()
    pattern = r"^u(?:16|32)\*\s*\w+\s*=.*?;"
    globals_once = re.findall(pattern, text, re.M)
    assert len(globals_once) == 5
    path.write_text(re.sub(pattern, lambda m: "extern " +
                          m[0].split("=")[0].strip() + ";", text, flags=re.M))
    path = output / "keypad.h"
    path.write_text(path.read_text().replace(
        "volatile u32* KEYS = (volatile u32*)0x04000130;",
        "#define KEYS ((volatile u32*)0x04000130)"))
    for filename, symbol in (("display.c", "vscreen[64000]"),
                             ("raster.c", "bscreen[62000]"),
                             ("vmachine.c", "cart[16384]")):
        path = output / filename
        path.write_text(path.read_text().replace(
            "BYTE " + symbol + ";",
            "BYTE " + symbol + ' __attribute__((section(".sbss")));'))

    path = output / "keyboard.c"
    text = path.read_text().replace("#define KEYS KEYZ", "")
    begin = text.index("if(!((*KEYS) & KEY_R))")
    end = text.index("\n\t}", begin) + 3
    path.write_text(text[:begin] + "/* Keep the bounded half-height display. */" + text[end:])
    path = output / "main.c"
    text = path.read_text().split("void * memcpy(")[0]
    text = text.replace("void C_Entry(void)", "int main(void)")
    text = text.replace("MODE_3 | BG2_ENABLE | OBJ_ENABLE | OBJ_MAP_1D", "MODE_3 | BG2_ENABLE")
    text = '#include "ui.h"\n' + text
    text = text.replace("loadCart(file);", "if (loadCart(file) != 0) atari_bad_rom();")
    path.write_text(text + "\n" + "\n".join(globals_once) + "\n")

    path = output / "cpu.c"
    text = path.read_text()
    old = "while (!(!((*KEYS) & KEY_L)))  {"
    assert old in text
    text = text.replace("register BYTE b;", "register BYTE b;\n  unsigned menu_poll = 0;")
    path.write_text('#include "ui.h"\n' + text.replace(old,
        "while (1) {\n"
        "  if ((menu_poll++ & 63) == 0 && ((~(*KEYS)) & 0x300) == 0x300 && atari_menu()) return;"))
    for name in ("ui.c", "ui.h", "timer.c", "timer.h"):
        shutil.copy2(ROOT / "tools/atari2600" / name, output / name)

    path = output / "vmachine.c"
    text = path.read_text()
    begin = text.index("void set_timer(")
    end = text.index("/* Main screen logic */", begin)
    text = text[:begin] + text[end:]
    reset = "timer_res=32;\n\ttimer_count=0;\n\ttimer_clks=0;"
    assert reset in text
    path.write_text('#include "timer.h"\n' + text.replace(reset, "atari_timer_reset();"))
    path = output / "memory.c"
    text = path.read_text()
    # WSYNC waits forward to the next horizontal blank, including vertical blank.
    old = "clk+=(ebeamx-tv_width)/3;"
    assert old in text
    text = text.replace(old, "/* Clock advance is shared below for every beam state. */")
    text = text.replace("\t\thbeam_state=HSYNCSTATE;\n\t\tebeamx=-tv_hsync;",
                        "\t\tif (ebeamx < tv_width) clk += (tv_width-ebeamx)/3;\n"
                        "\t\thbeam_state=HSYNCSTATE;\n\t\tebeamx=-tv_hsync;", 1)
    text = text.replace("case INTIM:\n\t\t\tcase 0x285:\n\t\t\tcase 0x29D:", "case INTIM:")
    needle = "\t\t\tcase SWCHA:"
    text = text.replace(needle, "\t\t\tcase 0x285:\n\t\t\tcase 0x29D:\n"
                        "\t\t\t\tres=do_timer_irq(clkcount);\n\t\t\t\tbreak;\n" + needle, 1)
    path.write_text('#include "timer.h"\n' + text)

    apply_raster_guards(output)

    # Replace the embedded-ROM selector with bounded runtime input.
    path = output / "files.c"
    notice = path.read_text().split("*/", 1)[0] + "*/\n"
    rom = diagnostic_rom()
    (output / "diagnostic.a26").write_bytes(rom)
    (output / "diagnostic-timer.a26").write_bytes(diagnostic_rom(timer=True))
    path.write_text(notice + (ROOT / "tools/atari2600/cartridge.c").read_text())
    shutil.copy2(ROOT / "tools/atari2600/startup.S", output / "startup.S")
    if fixture:
        (output / "diagnostic.c").write_text(
            'const unsigned char atari_diagnostic_rom[4096] '
            '__attribute__((aligned(4)))={' + ",".join(map(str, rom)) + '};\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path, help="clean pinned upstream clone")
    parser.add_argument("--out", type=Path, default=ROOT / "build/atari2600-prototype")
    parser.add_argument("--runtime", action="store_true", help="omit standalone diagnostic input")
    args = parser.parse_args()
    source, output = args.source.resolve(), args.out.resolve()
    assert source != output and output != ROOT
    commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=source, text=True).strip()
    assert commit == PIN, "Prototype expects the reviewed upstream revision"
    subprocess.run(["git", "diff", "--exit-code", "HEAD", "--", "."], cwd=source, check=True)
    output.mkdir(parents=True, exist_ok=True)
    prepare(source, output, not args.runtime)
    devkit = Path(os.environ.get("DEVKITARM", "/opt/devkitpro/devkitARM"))
    tools = Path(os.environ.get("DEVKITPRO", "/opt/devkitpro")) / "tools/bin"
    files = "main collisio cpu display files keyboard misc mouse raster table vmachine memory ui timer".split()
    command = [str(devkit / "bin/arm-none-eabi-gcc"), "-std=gnu89", "-O2", "-mthumb",
               "-mthumb-interwork", "-mcpu=arm7tdmi", "-ffunction-sections", "-fdata-sections",
               "-fwrapv", "-fno-strict-aliasing", "-fcommon", "-Wno-implicit-int",
               "-Wno-implicit-function-declaration", "-I.", "-specs=gba.specs", "-nostartfiles",
               *([] if args.runtime else ["-DA26_FIXTURE", "diagnostic.c"]), "startup.S",
               *[name + ".c" for name in files], "-Wl,--gc-sections,-Map=prototype.map",
               "-o", "prototype.elf"]
    with (output / "compile.log").open("w") as log:
        subprocess.run(command, cwd=output, stdout=log, stderr=subprocess.STDOUT, check=True)
    subprocess.run([str(devkit / "bin/arm-none-eabi-objcopy"), "-O", "binary",
                    "prototype.elf", "prototype.gba"], cwd=output, check=True)
    subprocess.run([str(tools / "gbafix"), "prototype.gba"], cwd=output, check=True)
    # Ship the exact normalized input and a standalone rebuild command alongside
    # local hardware trials; no old objects or embedded commercial ROMs are used.
    build_command = '"${DEVKITARM:-/opt/devkitpro/devkitARM}/bin/arm-none-eabi-gcc" ' + shlex.join(command[1:])
    (output / "build.sh").write_text(
        '#!/usr/bin/env bash\nset -euo pipefail\ncd "$(dirname "$0")"\n' + build_command + '\n' +
        '"${DEVKITARM:-/opt/devkitpro/devkitARM}/bin/arm-none-eabi-objcopy" -O binary prototype.elf prototype.gba\n' +
        '"${DEVKITPRO:-/opt/devkitpro}/tools/bin/gbafix" prototype.gba\n')
    shutil.copy2(ROOT / "COPYING", output / "COPYING")
    (output / "NOTICE.txt").write_text(
        'Experimental DSSMM Atari2600 plugin, based on MikeDX/2600_gba\n'
        'Upstream commit: ' + PIN + '\n'
        'Virtual VCS/X2600: Alex Hornby, Daniel Boris and the x64 contributors.\n'
        'See CREDITS and the notices retained in each source file.\n'
        'Upstream C sources state the GNU GPL without a numbered version;\n'
        'DSSMM adapters/menu are GPL-3.0-or-later. GPLv3 text is in COPYING.\n'
        'No upstream prebuilt objects or commercial ROM payloads are used.\n'
        'Rebuild this exact prepared source with bash build.sh and devkitARM.\n'
        'Experimental: no sound; only generated diagnostics tested.\n')
    record = {"upstream_commit": PIN, "purpose": "runtime input experiment" if args.runtime else "emulator-only generated diagnostic",
              "sha256": {name: hashlib.sha256((output / name).read_bytes()).hexdigest()
                         for name in ("diagnostic.a26", "prototype.gba")},
              "limits": "Experimental 2K/4K/F8/F6 input and L+R menu; audio, broad game compatibility, physical Exit remain pending."}
    (output / "prototype.json").write_text(json.dumps(record, indent=2) + "\n")
    print(output / "prototype.gba")


if __name__ == "__main__":
    main()
