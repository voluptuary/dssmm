#!/usr/bin/env python3
"""Pack sealed stock-size BLUE images; retain the historical vendor test recipe."""

import argparse
import hashlib
import importlib.util
from pathlib import Path
import zlib
import struct

RESOURCE_OFFSET = 0x100000
RESOURCE_SIZE = 1075676
RESOURCE_SHA256 = "be609b61939eaa19b97b81345f8e44e7c97b819fb76d230553d40f2d28688644"
FIRMWARE_OFFSET = 0x95000
FIRMWARE_SIZE = 0x716A0
FIRMWARE_CRC32 = 0x22DD74FA


def check_code(code):
    if not 0xC0 <= len(code) <= RESOURCE_OFFSET:
        raise ValueError("BLUE code must include a GBA header and fit below the 1 MiB resource boundary")


IMAGE_SIZE = 0x2069DC
SEAL_OFFSET = IMAGE_SIZE - 16
SEAL_MAGIC = b'DSSMBL1\0'


def pack(code, resource=None, elf=None):
    if elf is not None:
        checker_path = Path(__file__).with_name('check-blue-linked-layout.py')
        spec = importlib.util.spec_from_file_location('blue_linked_layout', checker_path)
        checker = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(checker)
        checker.validate(elf, code)
        if (code[0xA0:0xAC] != b'ezbluekernel' or code[0xB2] != 0x96 or
                code[3] != 0xEA or (sum(code[0xA0:0xBE]) + 0x19) & 255):
            raise ValueError('Invalid DSSMM BLUE cartridge header')
        result = bytearray(IMAGE_SIZE)
        result[:len(code)] = code
        crc = zlib.crc32(memoryview(result)[:SEAL_OFFSET])
        struct.pack_into('<8sII', result, SEAL_OFFSET, SEAL_MAGIC, crc, crc ^ 0xFFFFFFFF)
        return bytes(result)
    # Historical vendor recipe retained solely for baseline/resource regressions.
    check_code(code)
    if resource is None or len(resource) != RESOURCE_SIZE or hashlib.sha256(resource).hexdigest() != RESOURCE_SHA256:
        raise ValueError("BLUE resource differs from pinned vendor image.bin")
    firmware = resource[FIRMWARE_OFFSET:FIRMWARE_OFFSET + FIRMWARE_SIZE]
    if zlib.crc32(firmware) != FIRMWARE_CRC32:
        raise ValueError("BLUE firmware CRC does not match the vendor source")
    return code + bytes(RESOURCE_OFFSET - len(code)) + resource


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check-code", type=Path)
    parser.add_argument("--code", type=Path)
    parser.add_argument("--resource", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--elf", type=Path, help="Required for the current compact DSSMM BLUE image")
    args = parser.parse_args()
    try:
        if args.check_code:
            if args.code or args.resource or args.output or args.elf:
                parser.error("--check-code cannot be combined with packaging arguments")
            code = args.check_code.read_bytes()
            check_code(code)
            print(f"BLUE raw code: {len(code)} bytes; {RESOURCE_OFFSET - len(code)} before resource boundary")
            return
        if not (args.code and args.output and (args.elf or args.resource)):
            parser.error("supply --code, --elf and --output, or historical --resource")
        inputs = [args.code.resolve()]
        if args.resource:
            inputs.append(args.resource.resolve())
        if args.elf:
            inputs.append(args.elf.resolve())
        if args.output.resolve() in inputs:
            raise ValueError("output must not replace an input")
        result = pack(args.code.read_bytes(), args.resource.read_bytes() if args.resource else None, args.elf)
        # Never truncate an existing artifact or input after a failed check.
        with args.output.open("xb") as stream:
            stream.write(result)
        print(f"BLUE development image: {len(result)} bytes; hardware untested")
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
