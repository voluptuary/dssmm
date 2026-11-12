#!/usr/bin/env python3
"""Measure a DSSMM ROM layout around intact BLUE resources. Never emits an update."""

import argparse
import datetime
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import re
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[1]
ROM = 0x08000000
RESOURCE_START = 0x100000
ASSET_START = 0x207000
METADATA_START = 0x780000
ASSETS = re.compile(r"^(gImage_|acHZK12$|oem2uni936$|uni2oem936$|migoomba_gba$|launcher_.*_lz$|.*_raw$)")


def sections(elf, objdump):
    lines = subprocess.check_output([str(objdump), "-h", str(elf)], text=True).splitlines()
    result = []
    for index, line in enumerate(lines):
        match = re.match(r"\s*\d+\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+2\*\*", line)
        if match:
            name, *values = match.groups()
            result.append(dict(name=name, size=int(values[0], 16), vma=int(values[1], 16),
                               lma=int(values[2], 16), offset=int(values[3], 16), flags=lines[index + 1]))
    if not result:
        raise ValueError("No ELF sections found")
    return result


def measured_assets(elf, table, nm):
    data = elf.read_bytes()
    result = []
    for line in subprocess.check_output([str(nm), "-S", "--defined-only", str(elf)], text=True).splitlines():
        fields = line.split()
        if len(fields) != 4 or not ASSETS.match(fields[3]):
            continue
        address, size = int(fields[0], 16), int(fields[1], 16)
        section = next(s for s in table if s['vma'] <= address and address + size <= s['vma'] + s['size'])
        start = section['offset'] + address - section['vma']
        content = data[start:start + size]
        if len(content) != size:
            raise ValueError("Truncated asset")
        result.append((fields[3], size, hashlib.sha256(content).hexdigest()))
    if not result:
        raise ValueError("No artwork/font/audio assets found")
    return sorted(result)


def require_clear(start, end):
    if start < 0 or end > METADATA_START:
        raise ValueError("Load section exceeds the kernel area before save metadata")
    if start < ASSET_START and end > RESOURCE_START:
        raise ValueError("Load section overlaps protected BLUE resources/guard")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--donor', default='ezfode')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    donor = subprocess.check_output(['git', 'rev-parse', args.donor + '^{commit}'], cwd=ROOT, text=True).strip()
    devkit = Path(os.environ['DEVKITARM'])
    stock_path = devkit / 'arm-none-eabi/lib/gba_cart.ld'
    stock = stock_path.read_text()
    if stock.count('rom\t: ORIGIN = 0x08000000, LENGTH = 32M') != 1 or stock.count('\t.rodata :') != 1:
        raise ValueError('Toolchain linker script changed; review instead of guessing replacements')
    spec = importlib.util.spec_from_file_location('blue_packer', ROOT / 'scripts/pack-blue-kernel.py')
    packer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(packer)
    resource = (ROOT / 'targets/blue/resources/image.bin').read_bytes()
    packer.pack(bytes(192), resource)  # Validate pinned vendor resource and firmware CRC.
    # Negative boundary checks: reject one byte in resources or save metadata.
    for invalid in ((RESOURCE_START - 1, RESOURCE_START + 1), (ASSET_START - 1, ASSET_START + 1),
                    (METADATA_START, METADATA_START + 1)):
        try:
            require_clear(*invalid)
        except ValueError:
            continue
        raise AssertionError('Overlap check accepted a protected range')

    with tempfile.TemporaryDirectory(prefix='dssmm-blue-layout-') as folder:
        work = Path(folder)
        archive = subprocess.check_output(['git', 'archive', donor, 'Makefile', 'source', 'images'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(archive)) as source:
            for member in source.getmembers():
                destination = work / member.name
                if not destination.resolve().is_relative_to(work.resolve()):
                    raise ValueError('Archive path escapes temporary export')
                if member.isdir():
                    destination.mkdir(parents=True, exist_ok=True)
                elif member.isfile():
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    destination.write_bytes(source.extractfile(member).read())
                else:
                    raise ValueError('Unexpected non-file in donor export')

        def build(target):
            print(f'Building {target} from {donor[:12]}...', flush=True)
            process = subprocess.run(['make', 'BUILD=build-layout', 'TARGET=' + target], cwd=work,
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            if process.returncode:
                raise RuntimeError(process.stdout[-6000:])

        build('dssmm-baseline')
        layout_dir = work / 'layout'
        layout_dir.mkdir()
        linker = stock.replace('rom\t: ORIGIN = 0x08000000, LENGTH = 32M',
                               'rom\t: ORIGIN = 0x08000000, LENGTH = 0x780000')
        linker = linker.replace('\t.rodata :', '\t.rodata 0x08207000 :')
        linker += '''
ASSERT(ADDR(.fini) + SIZEOF(.fini) <= 0x08100000, "BLUE prefix overlaps resources")
ASSERT(ADDR(.rodata) >= 0x08207000, "BLUE assets overlap resources")
ASSERT(__rom_end__ <= 0x08780000, "BLUE ROM overlaps save metadata")
'''
        (layout_dir / 'gba_cart.ld').write_text(linker)
        makefile = work / 'Makefile'
        make = makefile.read_text()
        if make.count('LDFLAGS\t=\t-g') != 1 or make.count('KERNEL_SIZE_LIMIT := 2097152') != 1:
            raise ValueError('Donor build changed; review probe adaptation')
        make = make.replace('LDFLAGS\t=\t-g', f'LDFLAGS\t=\t-B{layout_dir}/ -g')
        make = make.replace('KERNEL_SIZE_LIMIT := 2097152', 'KERNEL_SIZE_LIMIT := 7864320')
        makefile.write_text(make.replace('2 MiB', 'BLUE pre-metadata area'))
        build('dssmm-scatter')
        nm, objdump = (devkit / 'bin' / ('arm-none-eabi-' + tool) for tool in ('nm', 'objdump'))
        baseline, scatter = work / 'dssmm-baseline.elf', work / 'dssmm-scatter.elf'
        before, after = sections(baseline, objdump), sections(scatter, objdump)
        assets = measured_assets(baseline, before, nm)
        if assets != measured_assets(scatter, after, nm):
            raise AssertionError('Linked artwork/font/audio asset content changed')
        ram = lambda table: [(s['name'], s['size'], s['vma']) for s in table if 0x02000000 <= s['vma'] < 0x04000000]
        if ram(before) != ram(after):
            raise AssertionError('RAM section size/address changed')
        image = (work / 'dssmm-scatter.gba').read_bytes()
        combined = bytearray(image)
        combined[RESOURCE_START:RESOURCE_START + len(resource)] = resource
        load_sections = []
        for section in after:
            if not section['size'] or not all(flag in section['flags'] for flag in ('CONTENTS', 'ALLOC', 'LOAD')):
                continue
            start, end = section['lma'] - ROM, section['lma'] - ROM + section['size']
            require_clear(start, end)
            if end > len(image) or image[start:end] != combined[start:end]:
                raise AssertionError('Resource placement changes a linked load section')
            load_sections.append(dict(name=section['name'], start=start, end=end))
        if combined[RESOURCE_START:RESOURCE_START + len(resource)] != resource:
            raise AssertionError('Vendor resource bytes changed')
        report = dict(donor_commit=donor, vendor_commit='109f1a050d9c35008379ce61ae40d05dc528c614',
                      measured_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      stock_linker_sha256=hashlib.sha256(stock.encode()).hexdigest(),
                      baseline_bytes=(work / 'dssmm-baseline.gba').stat().st_size,
                      layout_bytes=len(combined), asset_count=len(assets), asset_bytes=sum(a[1] for a in assets),
                      vendor_resource_sha256=hashlib.sha256(resource).hexdigest(),
                      resource_start=RESOURCE_START, resource_end=RESOURCE_START + len(resource),
                      asset_start=ASSET_START, metadata_start=METADATA_START,
                      remaining_before_metadata=METADATA_START - len(combined),
                      ram_sections=ram(after), load_sections=load_sections,
                      hardware_verified=False, updater_size_support_verified=False,
                      scope='Unmodified DE donor layout proof, not a functional BLUE port or flashable artifact')
        print(json.dumps(report, indent=2))
        if args.report:
            args.report.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
