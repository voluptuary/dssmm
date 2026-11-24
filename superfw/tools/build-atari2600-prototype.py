#!/usr/bin/env python3
"""Build a pinned, emulator-only Virtual VCS diagnostic outside firmware inputs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

PIN = "2eb18dd8a5b832c939efe510b4e493ad4cf25437"
ROOT = Path(__file__).resolve().parents[1]


def diagnostic_rom():
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
                          lambda m: m[1] + m[2].lower() + m[3], path.read_text())
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
    path = output / "display.c"
    path.write_text(path.read_text().replace("int color[255]", "int color[256]"))
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
    text = text.replace("loadCart(file);", "if (loadCart(file) != 0) {\n"
                        "  unsigned n; for(n=0;n<240*160;n++) VideoBuffer[n]=31;\n"
                        "  for(;;) {}\n}")
    path.write_text(text + "\n" + "\n".join(globals_once) + "\n")

    # Replace the embedded-ROM selector with bounded runtime input.
    path = output / "files.c"
    notice = path.read_text().split("*/", 1)[0] + "*/\n"
    rom = diagnostic_rom()
    (output / "diagnostic.a26").write_bytes(rom)
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
    files = "main collisio cpu display files keyboard misc mouse raster table vmachine memory".split()
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
    record = {"upstream_commit": PIN, "purpose": "runtime input experiment" if args.runtime else "emulator-only generated diagnostic",
              "sha256": {name: hashlib.sha256((output / name).read_bytes()).hexdigest()
                         for name in ("diagnostic.a26", "prototype.gba")},
              "limits": "Experimental input only; safe Exit, banked formats, audio, compatibility and physical qualification remain pending."}
    (output / "prototype.json").write_text(json.dumps(record, indent=2) + "\n")
    print(output / "prototype.gba")


if __name__ == "__main__":
    main()
