#!/usr/bin/env python3
"""Compare imported tracked files to pinned source commits without normalizing them."""
from pathlib import Path
import hashlib
import json
import subprocess
import sys
ROOT = Path(__file__).resolve().parents[1]
allowed_records = {'AGENTS.md', 'HANDOFF.md', 'PROGRESS.md'}
results = []
for target, record in json.loads((ROOT/'projects.json').read_text())['projects'].items():
    directory = ROOT/record['directory']
    if 'source_commit' not in record:
        results.append({'target':target,'status':'live hardware project; no imported snapshot pin'})
        continue
    source_prefix = record['source_directory'].rstrip('/') + '/'
    entries = subprocess.check_output(['git','-C',str(ROOT),'ls-tree','-rz',record['source_commit'],'--',source_prefix])
    if not entries:
        raise SystemExit('Pinned project directory is empty: '+target)
    checked = 0
    expected_names = set()
    for entry in entries.split(b'\0'):
        if not entry:
            continue
        metadata, name = entry.split(b'\t',1)
        mode, kind, expected = metadata.decode().split()
        name = name.decode()
        if not name.startswith(source_prefix):
            raise SystemExit("Source escaped project prefix")
        name = name[len(source_prefix):]
        expected_names.add(name)
        if name in allowed_records:
            continue
        if kind != 'blob' or mode not in ('100644','100755'):
            raise SystemExit('Unexpected source entry type: '+name)
        path = directory/name
        data = path.read_bytes()
        actual = hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()
        if actual != expected:
            # Git archive/checkouts honor eol attributes; compare the stored blob after Git's filters.
            filtered = subprocess.check_output(['git','-C',str(ROOT),'hash-object','--path='+str(path.relative_to(ROOT)),str(path)],text=True).strip()
            if filtered != expected:
                raise SystemExit('Snapshot differs from source commit: '+target+'/'+name)
        if bool(path.stat().st_mode & 0o111) != (mode == '100755'):
            raise SystemExit('Executable mode differs: '+target+'/'+name)
        checked += 1
    prefix = record['directory'] + '/'
    tracked = subprocess.check_output(['git','-C',str(ROOT),'ls-files','--',record['directory']],text=True).splitlines()
    extra = {name[len(prefix):] for name in tracked} - expected_names
    if extra:
        raise SystemExit('Unexpected tracked snapshot files: '+target+': '+', '.join(sorted(extra)))
    results.append({'target':target,'source_commit':record['source_commit'],'unchanged_files':checked})
print(json.dumps(results,indent=2))
if any('source_commit' in result for result in results):
    print('Imported source/assets/tools match Git blobs (including declared EOL filters); only local instructions/journals may differ.', file=sys.stderr)
else:
    print('Live hardware branch: no imported snapshot claim; run its project verifier.', file=sys.stderr)
