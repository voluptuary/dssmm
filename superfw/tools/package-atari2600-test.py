#!/usr/bin/env python3
"""Deliver an explicitly experimental Atari plugin and corresponding source locally."""
from pathlib import Path
import argparse
import hashlib
import json
import shutil
import zipfile

root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--build', type=Path, default=root / 'build/atari2600-runtime')
p.add_argument('--out', type=Path, default=root.parent / 'hardware-test')
a = p.parse_args()
build, out = a.build.resolve(), a.out.resolve()
assert build != out and build.is_dir()
record = json.loads((build / 'prototype.json').read_text())
assert record['purpose'] == 'runtime input experiment'
assert json.loads((build / 'input-validation.json').read_text())['result'] == 'passed'
fixture = build / 'runtime-fixture'
assert json.loads((fixture / 'validation.json').read_text())['runtime_sha256'] == record['sha256']['prototype.gba']
assert len(json.loads((fixture / 'menu-validation.json').read_text())['passed']) == 10
plugin = out / 'SYSTEM/PLUG/a26.gba'
plugin.parent.mkdir(parents=True, exist_ok=True)
shutil.copy2(build / 'prototype.gba', plugin)
assert hashlib.sha256(plugin.read_bytes()).hexdigest() == record['sha256']['prototype.gba']
# Exact prepared source, headers and reproducible build. No objects, ROMs, BIOS,
# scratch data or upstream Windows binaries. This local source ZIP is not a
# firmware release/source asset and is never uploaded by the release workflow.
source = out / 'atari2600-test-source.zip'
with zipfile.ZipFile(source, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
    for path in sorted(build.iterdir()):
        if path.is_file() and (path.suffix in ('.c', '.h', '.S') or
                path.name in ('build.sh', 'COPYING', 'NOTICE.txt', 'CREDITS', 'readme.txt')):
            if path.name in ('diagnostic.c', 'runtime-seed.S'):
                continue
            archive.write(path, 'atari2600/' + path.name)
(out / 'atari2600-test.txt').write_text('''EXPERIMENTAL Atari2600 plugin for SuperFW DSSMM

Copy SYSTEM/PLUG/a26.gba from this folder to SYSTEM/PLUG/a26.gba on your SD card.
Select an .a26 ROM normally. Build799 already recognizes extension-named plugins;
no replacement firmware is needed. Remove any older a26.bin: .bin takes priority.

L+R: open menu. D-pad: navigate/change. A: select. B: back/resume.
Menu: Controller, Display, Other Settings, Help, Resume, Restart, Exit.
Start resets the Atari console; Select operates its game-select switch.

Supported input layouts: 2K, 4K, standard 8K F8 and 16K F6.
Other banking variants are not supported. There is currently NO SOUND.
This unfinished core is an experimental trial, not a compatibility promise.
Only original generated diagnostics have been tested in mGBA; actual games,
performance and physical-card Exit remain unqualified. No games are included.
If return is unavailable, the plugin displays a power-off message.

Based on MikeDX/2600_gba and Virtual VCS/X2600 by Alex Hornby, Daniel Boris
and contributors listed in CREDITS. DSSMM startup, loader and menu additions.
Corresponding prepared source, notices, GPL text and build instructions are in
atari2600-test-source.zip. No upstream prebuilt emulator objects were used.
''')
record['hardware_test'] = 'Experimental local trial only; no physical qualification.'
record['assets'] = {str(path.relative_to(out)): hashlib.sha256(path.read_bytes()).hexdigest()
                    for path in (plugin, source)}
record['runtime_checks'] = json.loads((fixture / 'menu-validation.json').read_text())
(out / 'atari2600-test.json').write_text(json.dumps(record, indent=2) + '\n')
print(plugin)
