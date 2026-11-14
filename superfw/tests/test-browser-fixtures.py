#!/usr/bin/env python3
"""Check synthetic ROM headers and their filename/code-matched artwork."""
import pathlib
import struct
import subprocess
import sys
import tempfile

project_root = pathlib.Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="superfw-browser-fixtures-") as temp:
    root = pathlib.Path(temp) / "fixtures"
    header = pathlib.Path(temp) / "browser_fixture_assets.h"
    subprocess.run([
        sys.executable,
        str(project_root / "scripts/generate-browser-fixtures.py"),
        str(root), "--header", str(header),
    ], check=True, stdout=subprocess.DEVNULL)
    roms = sorted((root / "Game Boy Advance").rglob("*.gba"))
    assert len(roms) == 3
    for rom in roms:
        raw = rom.read_bytes()
        code = raw[0xAC:0xB0].decode("ascii")
        assert len(raw) == 32768 and code.isalnum()
        assert raw[0xBD] == (-sum(raw[0xA0:0xBD]) - 0x19) & 0xFF
        title = root / "SYSTEM" / "IMGS" / code[0] / code[1] / f"{code}.bmp"
        box = (root / "SYSTEM" / "IMGS2" / "Game Boy Advance" /
               rom.relative_to(root / "Game Boy Advance").with_suffix(".bmp"))
        for path, expected_width in ((title, 120), (box, 80)):
            data = path.read_bytes()
            assert data[:2] == b"BM"
            assert (data[6:10] == b"DST1") == (path != title or code != "FAKE")
            assert struct.unpack_from("<i", data, 18)[0] == expected_width
            assert struct.unpack_from("<i", data, 22)[0] == -80
            assert struct.unpack_from("<I", data, 2)[0] == len(data)
    generated = header.read_text()
    assert "browser_fixture_codes[3][5]" in generated
    assert all(f'"{code}"' in generated for code in ("FAKE", "PXL1", "ORBT"))
print("Synthetic GBA header codes and matching Title/Box BMP fixtures passed.")
