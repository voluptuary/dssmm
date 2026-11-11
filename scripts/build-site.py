#!/usr/bin/env python3
"""Compose an explicit Pages artifact without sharing mutable target sources."""
import json
from pathlib import Path
import shutil
ROOT = Path(__file__).resolve().parents[1]
out = ROOT / '.site'
if out.exists():
    shutil.rmtree(out)
shutil.copytree(ROOT / 'docs', out)
for target, record in json.loads((ROOT / 'projects.json').read_text())['projects'].items():
    docs = ROOT / record['directory'] / 'docs'
    if docs.is_dir():
        shutil.copytree(docs, out / target)
