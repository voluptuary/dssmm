#!/usr/bin/env python3
"""Exercise the actual Atari core's L+R menu in stock SDL mGBA."""
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from PIL import ImageGrab


def main():
    if os.environ.get('DSSMM_ATARI_MENU_DISPLAY') != '1':
        subprocess.run(['xvfb-run', '-a', '-s', '-screen 0 1024x768x24',
                        sys.executable, __file__, *sys.argv[1:]],
                       env=dict(os.environ, DSSMM_ATARI_MENU_DISPLAY='1'), check=True)
        return
    output = Path(sys.argv[1] if len(sys.argv) > 1 else
                  Path(__file__).resolve().parents[1] / 'build/atari2600-prototype').resolve()
    with tempfile.TemporaryDirectory(prefix='dssmm-atari-menu-') as work:
        folder = Path(work)
        # A marker replaces only cartridge remapping: the plugin's actual Exit
        # validation/jump runs unchanged. Firmware restoration has separate tests.
        marker_source = folder / 'return.S'
        marker_source.write_text(""".arm
.global _start
_start:
 b entry
 .word 0x54525344
entry:
 mov r0,#0x06000000
 mov r1,#0x3e0
 mov r2,#38400
1: strh r1,[r0],#2
 subs r2,r2,#1
 bne 1b
2: b 2b
""")
        devkit = Path(os.environ.get('DEVKITARM', '/opt/devkitpro/devkitARM')) / 'bin'
        subprocess.run([str(devkit / 'arm-none-eabi-as'), '-mcpu=arm7tdmi',
                        str(marker_source), '-o', str(folder / 'return.o')], check=True)
        subprocess.run([str(devkit / 'arm-none-eabi-objcopy'), '-O', 'binary',
                        str(folder / 'return.o'), str(folder / 'return.bin')], check=True)
        image = bytearray((output / 'prototype.gba').read_bytes())
        image.extend(bytes(0x1a00000 - len(image)))
        struct.pack_into('<2I', image, 0x18000a0, 0x45505553, 0x00574652)
        struct.pack_into('<I', image, 0x18000cc, 512)
        image.extend((folder / 'return.bin').read_bytes())
        (folder / 'test.gba').write_bytes(image)
        env = dict(os.environ, SDL_AUDIODRIVER='dummy', XDG_CONFIG_HOME=str(folder / 'config'))
        with (output / 'menu-mgba.log').open('w') as log:
            emulator = subprocess.Popen([os.environ.get('MGBA_BIN', '/usr/games/mgba'),
                                         '-1', 'test.gba'], cwd=folder, env=env,
                                        stdout=log, stderr=subprocess.STDOUT)
            try:
                time.sleep(5)
                windows = subprocess.check_output(['xdotool', 'search', '--onlyvisible',
                                                  '--pid', str(emulator.pid)], text=True).split()
                window = next(w for w in windows if 'WIDTH=240\nHEIGHT=160' in
                              subprocess.check_output(['xdotool', 'getwindowgeometry', '--shell', w], text=True))
                subprocess.run(['xdotool', 'windowmove', '--sync', window, '0', '0'], check=True)

                def capture(name):
                    time.sleep(.3)
                    assert emulator.poll() is None
                    image = ImageGrab.grab(bbox=(0, 0, 240, 160), xdisplay=os.environ['DISPLAY']).convert('RGB')
                    image.save(output / ('menu-' + name + '.png'))
                    return image

                def key(*keys):
                    subprocess.run(['xdotool', 'keydown', *keys], check=True)
                    time.sleep(.16)
                    subprocess.run(['xdotool', 'keyup', *keys], check=True)
                    time.sleep(.16)

                game = capture('game')
                key('a')  # L alone must not reset or open the menu.
                assert capture('l-alone').tobytes() == game.tobytes()
                key('a', 's')
                menu = capture('main')
                assert menu.tobytes() != game.tobytes()
                assert menu.getpixel((35, 110)) == (0, 0, 0)
                key('x')
                controller = capture('controller')
                assert controller.tobytes() != menu.tobytes()
                key('x')
                assert capture('controller-changed').tobytes() != controller.tobytes()
                key('z')
                assert capture('back').tobytes() == menu.tobytes()
                key('Down')
                key('x')
                display = capture('display')
                key('Left')
                assert capture('display-changed').tobytes() != display.tobytes()
                key('z')
                key('z')
                assert capture('resumed').tobytes() == game.tobytes()
                key('a', 's')
                for _ in range(5):
                    key('Down')
                key('x')
                time.sleep(1)
                assert capture('restarted').tobytes() == game.tobytes()
                key('a', 's')
                for _ in range(6):
                    key('Down')
                key('x')
                exited = capture('exited')
                assert set(exited.getdata()) == {(0, 255, 0)}
                (output / 'menu-validation.json').write_text(json.dumps({
                    'emulator': 'stock SDL mGBA 0.10.1; private Xvfb; no BIOS',
                    'passed': ['L alone leaves game running', 'L+R opens menu',
                               'A opens Controller', 'A toggles controller option',
                               'B returns to parent', 'D-pad selects Display',
                               'Left changes frame skip', 'B restores exact game frame',
                               'Restart returns to diagnostic', 'Exit reaches validated return marker'],
                    'limits': 'Original generated 4K diagnostic only; Return marker substitutes physical firmware remapping; game compatibility and audio unqualified.'
                }, indent=2) + '\n')
                print('Atari menu: 10 actual mGBA interaction checks passed.')
            finally:
                emulator.terminate()
                try:
                    emulator.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    emulator.kill()
                    emulator.wait()


if __name__ == '__main__':
    main()
