#!/usr/bin/env python3
"""Test real vendor resources, fixed offsets, collisions and non-destructive CLI failures."""

import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import struct
import zlib

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/pack-blue-kernel.py"
RESOURCE = ROOT / "targets/blue/resources/image.bin"
spec = importlib.util.spec_from_file_location("blue_packer", SCRIPT)
packer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packer)


class PackagingTest(unittest.TestCase):
    def test_vendor_layout_and_firmware_addresses(self):
        code = bytes(range(256))
        resource = RESOURCE.read_bytes()
        result = packer.pack(code, resource)
        self.assertEqual(len(result), 2124252)
        self.assertEqual(result[:256], code)
        self.assertEqual(result[256:0x100000], bytes(0x100000 - 256))
        self.assertEqual(result[0x100000:], resource)
        self.assertEqual(result[0x195000:0x2066A0], resource[0x95000:0x1066A0])

    def test_actual_compact_layout(self):
        elf = ROOT / "targets/blue/ezbluekernel-development.elf"
        code = elf.with_suffix(".gba").read_bytes()
        result = packer.pack(code, elf=elf)
        self.assertEqual(len(result), 2124252)
        self.assertEqual(result[:len(code)], code)
        self.assertEqual(result[len(code):packer.SEAL_OFFSET], bytes(packer.SEAL_OFFSET-len(code)))
        magic, crc, inverse = struct.unpack_from('<8sII', result, packer.SEAL_OFFSET)
        self.assertEqual(magic, packer.SEAL_MAGIC)
        self.assertEqual(crc, zlib.crc32(result[:packer.SEAL_OFFSET]))
        self.assertEqual(inverse, crc ^ 0xffffffff)
        firmware = RESOURCE.read_bytes()[packer.FIRMWARE_OFFSET:packer.FIRMWARE_OFFSET+packer.FIRMWARE_SIZE]
        self.assertNotIn(firmware, result)
        for offset in (0x220, 0x40000, len(code)-1):
            damaged = bytearray(code); damaged[offset] ^= 1
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                packer.pack(bytes(damaged), elf=elf)
        for invalid in (code[:-1], code+b'x'):
            with self.assertRaises(ValueError): packer.pack(invalid, elf=elf)

    def test_boundary(self):
        resource = RESOURCE.read_bytes()
        self.assertEqual(packer.pack(bytes(0x100000), resource)[0x100000:], resource)
        for size in (0, 191, 0x100001, 0x200000):
            with self.subTest(size=size), self.assertRaises(ValueError):
                packer.pack(bytes(size), resource)

    def test_corrupt_or_truncated_resources(self):
        resource = RESOURCE.read_bytes()
        for value in (resource[:-1], resource + b"\0", b"X" + resource[1:]):
            with self.assertRaises(ValueError):
                packer.pack(bytes(192), value)

    def test_cli_preserves_inputs_and_existing_outputs(self):
        with tempfile.TemporaryDirectory() as folder:
            code = Path(folder) / "test.gba"
            code.write_bytes(bytes(192))
            for output in (code, RESOURCE):
                proc = subprocess.run([sys.executable, str(SCRIPT), "--code", str(code),
                                       "--resource", str(RESOURCE), "--output", str(output)],
                                      capture_output=True)
                self.assertNotEqual(proc.returncode, 0)
            self.assertEqual(code.read_bytes(), bytes(192))
            destination = Path(folder) / "test.bin"
            command = [sys.executable, str(SCRIPT), "--code", str(code),
                       "--resource", str(RESOURCE), "--output", str(destination)]
            subprocess.run(command, check=True, capture_output=True)
            first = destination.read_bytes()
            self.assertNotEqual(subprocess.run(command, capture_output=True).returncode, 0)
            self.assertEqual(destination.read_bytes(), first)


if __name__ == "__main__":
    unittest.main()
