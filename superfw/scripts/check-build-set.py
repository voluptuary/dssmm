#!/usr/bin/env python3
"""Validate identity and integrity of an existing three-board candidate set."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
from firmware_limits import check_size

root = Path(__file__).resolve().parents[1]
records = []
for board in ('sd', 'lite', 'chis'):
    folder = root / 'build' / board
    record = json.loads((folder / 'build-info.json').read_text())
    image = (folder / 'superfw.fw').read_bytes()
    sizes=check_size(board,len(image))
    for field,value in sizes.items():
        assert record.get(field,value)==value,f'{board}: incorrect size manifest {field}'
    record.update(sizes)
    assert record['board'] == board, f'{board}: wrong manifest board'
    assert record['source_dirty'] is False, f'{board}: modified source build'
    assert hashlib.sha256(image).hexdigest() == record['firmware_sha256'], f'{board}: image hash mismatch'
    assert len(image) >= 0xF0 and len(image) % 512 == 0, f'{board}: invalid size'
    assert struct.unpack_from('<I', image, 0xCC)[0] == len(image), f'{board}: embedded size mismatch'
    assert (sum(image[0xA0:0xBE]) + 0x19) & 255 == 0, f'{board}: GBA header checksum'
    sealed = bytearray(image)
    sealed[0xE0:0xF0] = bytes(16)
    assert hashlib.sha256(sealed).digest()[:16] == image[0xE0:0xF0], f'{board}: firmware seal'
    count = int(subprocess.check_output(['git', 'rev-list', '--count', record['source_commit']], cwd=root))
    assert count == record['dssmm_build'], f'{board}: build number does not match commit'
    records.append(record)
for field in ('source_commit', 'dssmm_build', 'source_tree_sha256'):
    assert len({record[field] for record in records}) == 1, f'mixed candidate {field}'
print(json.dumps({'validation': 'candidate identity and integrity passed; hardware not qualified',
                  'build': records[0]['dssmm_build'], 'boards': records}, indent=2))
