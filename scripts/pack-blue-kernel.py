#!/usr/bin/env python3
"""Reproduce the pinned BLUE vendor packer's layout; not hardware qualification."""

import argparse
import hashlib
from pathlib import Path
import zlib

RESOURCE_OFFSET = 0x100000
RESOURCE_SIZE = 1075676
RESOURCE_SHA256 = "be609b61939eaa19b97b81345f8e44e7c97b819fb76d230553d40f2d28688644"
FIRMWARE_OFFSET = 0x95000
FIRMWARE_SIZE = 0x716A0
FIRMWARE_CRC32 = 0x22DD74FA


def check_code(code):
    if not 0xC0 <= len(code) <= RESOURCE_OFFSET:
        raise ValueError("BLUE code must include a GBA header and fit below the 1 MiB resource boundary")


def pack(code, resource):
    check_code(code)
    if len(resource) != RESOURCE_SIZE or hashlib.sha256(resource).hexdigest() != RESOURCE_SHA256:
        raise ValueError("BLUE resource differs from pinned vendor image.bin")
    firmware = resource[FIRMWARE_OFFSET:FIRMWARE_OFFSET + FIRMWARE_SIZE]
    if zlib.crc32(firmware) != FIRMWARE_CRC32:
        raise ValueError("BLUE firmware CRC does not match the vendor source")
    # The vendor uses fseek before fwrite: the intervening file hole reads as zero.
    return code + bytes(RESOURCE_OFFSET - len(code)) + resource


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check-code", type=Path)
    parser.add_argument("--code", type=Path)
    parser.add_argument("--resource", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    try:
        if args.check_code:
            if args.code or args.resource or args.output:
                parser.error("--check-code cannot be combined with packaging arguments")
            code = args.check_code.read_bytes()
            check_code(code)
            print(f"BLUE raw code: {len(code)} bytes; {RESOURCE_OFFSET - len(code)} before resource boundary")
            return
        if not (args.code and args.resource and args.output):
            parser.error("supply --code, --resource and --output, or --check-code")
        if args.output.resolve() in (args.code.resolve(), args.resource.resolve()):
            raise ValueError("output must not replace an input")
        result = pack(args.code.read_bytes(), args.resource.read_bytes())
        # Never truncate an existing artifact or input after a failed check.
        with args.output.open("xb") as stream:
            stream.write(result)
        print(f"BLUE development image: {len(result)} bytes; hardware untested")
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
