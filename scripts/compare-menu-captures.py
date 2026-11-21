#!/usr/bin/env python3
"""Make a portable side-by-side/overlay report from native 240x160 PNG captures."""
import argparse,base64,html,struct
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('reference',type=Path);p.add_argument('candidate',type=Path)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--reference-label',default='EZ-FLASH reference')
p.add_argument('--candidate-label',default='SuperFW candidate')
a=p.parse_args()
def uri(path):
 data=path.read_bytes()
 if data[:8]!=b'\x89PNG\r\n\x1a\n' or struct.unpack('>II',data[16:24])!=(240,160):
  p.error(f'{path}: expected native 240x160 PNG')
 return 'data:image/png;base64,'+base64.b64encode(data).decode()
r,c=uri(a.reference),uri(a.candidate)
page='''<!doctype html><meta charset="utf-8"><title>Native GBA menu comparison</title>
<style>body{font:16px system-ui;background:#eee;color:#222;margin:2em}main{display:flex;gap:2em;flex-wrap:wrap}img{width:480px;height:320px;image-rendering:pixelated}figure{margin:0}#overlay{position:relative;width:480px;height:320px}#candidate{position:absolute;inset:0;opacity:.5}label{display:block;margin:1em 0}</style>
<h1>Native GBA menu comparison</h1><p>Renderer captures with deterministic fixtures. Visual evidence only; card timing, storage and gameplay are not emulated.</p>
<main><figure><figcaption>REF</figcaption><img src="RURI"></figure><figure><figcaption>CAND</figcaption><img src="CURI"></figure></main>
<label>Candidate overlay opacity <input type="range" min="0" max="100" value="50" oninput="document.getElementById('candidate').style.opacity=this.value/100"></label>
<div id="overlay"><img src="RURI"><img id="candidate" src="CURI"></div>
'''.replace('REF',html.escape(a.reference_label)).replace('CAND',html.escape(a.candidate_label)).replace('RURI',r).replace('CURI',c)
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(page)
print(a.output)
