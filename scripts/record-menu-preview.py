#!/usr/bin/env python3
"""Record native mGBA menu fixtures as README GIFs, with reproducible input notes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
from PIL import Image, ImageGrab


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--frames", type=int, default=100)
    parser.add_argument("--fps", type=int, default=20)
    parser.add_argument("--keys", default="Right,Right,Left")
    args = parser.parse_args()
    if os.environ.get("DSSMM_PRIVATE_GIF_DISPLAY") != "1":
        env = dict(os.environ, DSSMM_PRIVATE_GIF_DISPLAY="1")
        subprocess.run(["xvfb-run", "-a", "-s", "-screen 0 1024x768x24", sys.executable, __file__, *sys.argv[1:]], env=env, check=True)
        return
    assert args.frames > 0 and 1 <= args.fps <= 30
    rom = args.rom.resolve()
    args.out.parent.mkdir(parents=True, exist_ok=True)
    keys = args.keys.split(",") if args.keys else []
    events = {args.frames * (i + 1) // (len(keys) + 1): key for i, key in enumerate(keys)}
    frames, timings = [], []
    releases = {index + 2: key for index, key in events.items()}
    with tempfile.TemporaryDirectory(prefix="dssmm-menu-gif-") as work:
        folder = Path(work)
        shutil.copy2(rom, folder / "preview.gba")
        env = dict(os.environ, SDL_AUDIODRIVER="dummy", XDG_CONFIG_HOME=str(folder / "config"))
        with (folder / "mgba.log").open("w") as log:
            emulator = subprocess.Popen([os.environ.get("MGBA_BIN", "/usr/games/mgba"),
                                         "-1", "preview.gba"], cwd=folder, env=env,
                                        stdout=log, stderr=subprocess.STDOUT)
            try:
                time.sleep(5)
                windows = subprocess.check_output(["xdotool", "search", "--onlyvisible", "--pid", str(emulator.pid)], text=True).split()
                bounds = None
                for window in windows:
                    fields = dict(line.split("=", 1) for line in subprocess.check_output(
                        ["xdotool", "getwindowgeometry", "--shell", window], text=True).splitlines())
                    if (int(fields["WIDTH"]), int(fields["HEIGHT"])) == (240, 160):
                        subprocess.run(["xdotool", "windowmove", "--sync", window, "0", "0"], check=True)
                        bounds = (0, 0, 240, 160)
                        break
                assert bounds, "Native1x mGBA window not found"
                start = time.monotonic()
                for index in range(args.frames):
                    assert emulator.poll() is None, "mGBA stopped before capture completed"
                    if index in releases:
                        subprocess.run(["xdotool", "keyup", releases[index]], check=True)
                    if index in events:
                        subprocess.run(["xdotool", "keydown", events[index]], check=True)
                    image = ImageGrab.grab(bbox=bounds, xdisplay=os.environ["DISPLAY"]).convert("RGB")
                    assert image.size == (240, 160)
                    frames.append(image)
                    timings.append(round(time.monotonic() - start, 4))
                    time.sleep(max(0, (index + 1) / args.fps - (time.monotonic() - start)))
            finally:
                emulator.terminate()
                try:
                    emulator.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    emulator.kill()
                    emulator.wait()
    assert len(set(image.tobytes() for image in frames)) > 1 or not keys, "Input produced no visible change"
    samples = frames[::max(1, len(frames) // 16)]
    sheet = Image.new("RGB", (240, 160 * len(samples)))
    for index, frame in enumerate(samples):
        sheet.paste(frame, (0, index * 160))
    palette = sheet.quantize(colors=256)
    encoded = [frame.quantize(palette=palette, dither=Image.Dither.NONE)
               .resize((480, 320), Image.Resampling.NEAREST) for frame in frames]
    encoded[0].save(args.out, save_all=True, append_images=encoded[1:],
                    duration=round(1000 / args.fps), loop=0, optimize=True, disposal=1)
    record = {"rom_sha256": hashlib.sha256(rom.read_bytes()).hexdigest(),
              "gif_sha256": hashlib.sha256(args.out.read_bytes()).hexdigest(),
              "frames_captured": len(frames), "capture_times_seconds": timings,
              "inputs": events, "gif_frame_duration_ms": round(1000 / args.fps),
              "renderer": "stock SDL mGBA0.10.1, native menu fixture; no BIOS supplied",
              "capture": "X11 native1x window framebuffer; no screenshot hotkey stalls",
              "conversion": "Pillow; shared adaptive palette, nearest-neighbor2x, no interpolation",
              "limits": "Synthetic artwork/storage. Showcase, not cartridge timing or hardware qualification."}
    args.out.with_suffix(".json").write_text(json.dumps(record, indent=2) + "\n")
    print(args.out)


if __name__ == "__main__":
    main()
