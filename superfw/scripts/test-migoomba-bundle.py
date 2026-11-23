#!/usr/bin/env python3
"""Reject an incorrect/corrupt or hardware-specific bundled GB/GBC payload."""
from pathlib import Path
import json,hashlib,struct
root=Path(__file__).resolve().parents[1]
r=json.loads((root/'docs/plugins/migoomba.json').read_text());data=(root/r['payload']).read_bytes()
assert r['variant']=='generic' and r['upstream_commit']=='4b401e2ac7876aed9f89face867fd43d89ce189d'
assert len(data)==r['bytes']==81892 and len(data)%4==0
assert hashlib.sha256(data).hexdigest()==r['sha256']=='a2e3fa5abd47d7b9318d879af712524d4677807239c439f57b951b0047f1def3'
assert data[0xb2]==0x96 and (sum(data[0xa0:0xbe])+0x19)&255==0
entry=struct.unpack_from('<I',data)[0];assert entry&0xff000000==0xea000000
print('Pinned MiGoomba v0.1 generic: exact payload/header/alignment passed; no DE hardware variant.')
