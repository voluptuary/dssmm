#!/usr/bin/env python3
"""Generate disposable, synthetic GBA headers and artwork for browser tests."""
from __future__ import annotations

import argparse
import pathlib
import struct

GAMES = (
    ("Fake Quest (USA).gba", "FAKE", "Fake Quest"),
    ("Pixel Garden (USA).gba", "PXL1", "Pixel Garden"),
    ("Orbit Racer (USA).gba", "ORBT", "Orbit Racer"),
)


def bmp(width: int, code: str, salt: int, marked: bool = True) -> bytes:
    height = 80
    pixels = bytearray()
    for y in range(height):
        for x in range(width):
            # RGB555 geometric patterns that remain distinct when reduced.
            r = (x // max(1, width // 5) + salt) % 32
            g = (y // 16 + salt * 3) % 32
            b = ((x // 8) ^ (y // 8) ^ salt) % 32
            color = r | (g << 5) | (b << 10)
            if marked and (x + y + salt * 7) % 29 < 3:
                color = 0  # DST1 transparency marker is enabled.
            pixels += struct.pack("<H", color)
    image_size = len(pixels)
    header = bytearray(54)
    header[0:2] = b"BM"
    struct.pack_into("<I", header, 2, 54 + image_size)
    if marked:
        header[6:10] = b"DST1"
    struct.pack_into("<I", header, 10, 54)
    struct.pack_into("<I", header, 14, 40)
    struct.pack_into("<iiHHII", header, 18, width, -height, 1, 16, 0, image_size)
    return bytes(header) + bytes(pixels)


def gba_fixture(title: str, game_code: str) -> bytes:
    # A 32 KiB non-launchable browser fixture. It has a populated GBA header
    # and correct header checksum; there is no copyrighted boot logo/code.
    rom = bytearray(32 * 1024)
    title_bytes = title.encode("ascii")[:12].ljust(12, b" ")
    rom[0xA0:0xAC] = title_bytes
    rom[0xAC:0xB0] = game_code.encode("ascii")
    rom[0xB0:0xB2] = b"01"
    rom[0xB2] = 0x96
    rom[0xBC] = 0
    rom[0xBD] = (-sum(rom[0xA0:0xBD]) - 0x19) & 0xFF
    return bytes(rom)


def c_array(name: str, data: bytes) -> str:
    lines = [f"static const uint8_t {name}[{len(data)}] = {{"]
    for i in range(0, len(data), 16):
        lines.append("  " + ", ".join(f"0x{x:02x}" for x in data[i:i + 16]) + ",")
    lines.append("};")
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output", type=pathlib.Path)
    parser.add_argument("--header", type=pathlib.Path)
    args = parser.parse_args()
    root = args.output
    root.mkdir(parents=True, exist_ok=True)
    title_images = []
    box_images = []
    rom_headers = []
    for index, (filename, code, title) in enumerate(GAMES):
        folder = filename[0].upper()
        rom = gba_fixture(title, code)
        rom_path = root / "Game Boy Advance" / folder / filename
        rom_path.parent.mkdir(parents=True, exist_ok=True)
        rom_path.write_bytes(rom)

        title_image = bmp(120, code, index + 1, marked=index != 0)
        box_image = bmp(80, code, index + 4)
        title_images.append(title_image)
        box_images.append(box_image)
        code_path = root / "SYSTEM" / "IMGS" / code[0] / code[1] / f"{code}.bmp"
        code_path.parent.mkdir(parents=True, exist_ok=True)
        code_path.write_bytes(title_image)
        box_path = root / "SYSTEM" / "IMGS2" / "Game Boy Advance" / folder / (filename[:-4] + ".bmp")
        box_path.parent.mkdir(parents=True, exist_ok=True)
        box_path.write_bytes(box_image)
        rom_headers.append(rom[:0xBE])

    if args.header:
        arrays = []
        for index, data in enumerate(rom_headers):
            arrays.append(c_array(f"fixture_rom_header_{index}", data))
        for index, data in enumerate(title_images):
            arrays.append(c_array(f"fixture_title_bmp_{index}", data))
        for index, data in enumerate(box_images):
            arrays.append(c_array(f"fixture_box_bmp_{index}", data))
        meta = [
            "typedef struct { const uint8_t *rom; const uint8_t *title; const uint8_t *box; } browser_fixture;",
            "static const browser_fixture browser_fixtures[3] = {",
        ]
        meta += [f"  {{fixture_rom_header_{i}, fixture_title_bmp_{i}, fixture_box_bmp_{i}}},"
                 for i in range(len(GAMES))]
        meta.append("};")
        meta.append("static const char browser_fixture_codes[3][5] = {" +
                    ", ".join(f'"{code}"' for _, code, _ in GAMES) + "};")
        meta.append("static const char *const browser_fixture_names[3] = {" +
                    ", ".join('"' + filename.replace('"', '\\"') + '"' for filename, _, _ in GAMES) + "};")
        args.header.parent.mkdir(parents=True, exist_ok=True)
        args.header.write_text("#ifndef BROWSER_FIXTURE_ASSETS_H\n#define BROWSER_FIXTURE_ASSETS_H\n#include <stdint.h>\n" +
                               "\n".join(arrays + meta) + "\n#endif\n")

    print(f"Generated {len(GAMES)} synthetic ROM headers and matching title/box artwork in {root}")


if __name__ == "__main__":
    main()
