#!/usr/bin/env python3
"""Join real renderer GIF clips or still captures without inventing transitions."""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageSequence

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("--out", required=True, type=Path)
p.add_argument("--hold-ms", type=int, default=1400)
p.add_argument("inputs", nargs="+", type=Path)
a = p.parse_args()
frames, durations, sources = [], [], []
for path in a.inputs:
    image = Image.open(path)
    animated = image.format == "GIF" and getattr(image, "is_animated", False)
    for frame in ImageSequence.Iterator(image):
        assert frame.size in ((240, 160), (480, 320)), (path, frame.size)
        durations.append(frame.info.get("duration", 50) if animated else a.hold_ms)
        frames.append(frame.convert("RGB").resize((480, 320), Image.Resampling.NEAREST))
    sources.append({"name": path.name, "sha256": hashlib.sha256(path.read_bytes()).hexdigest()})
samples = frames[::max(1, len(frames) // 24)]
sheet = Image.new("RGB", (480, 320 * len(samples)))
for index, frame in enumerate(samples):
    sheet.paste(frame, (0, index * 320))
palette = sheet.quantize(colors=256)
encoded = [frame.quantize(palette=palette, dither=Image.Dither.NONE) for frame in frames]
a.out.parent.mkdir(parents=True, exist_ok=True)
encoded[0].save(a.out, save_all=True, append_images=encoded[1:], duration=durations,
                loop=0, disposal=1, optimize=True)
record = {"sources": sources, "size": [480, 320], "duration_ms": sum(durations),
          "sha256": hashlib.sha256(a.out.read_bytes()).hexdigest(),
          "presentation": "Native menu captures; nearest-neighbor2x, no generated in-between frames"}
a.out.with_suffix(".json").write_text(json.dumps(record, indent=2) + "\n")
print(a.out, a.out.stat().st_size, "bytes")
