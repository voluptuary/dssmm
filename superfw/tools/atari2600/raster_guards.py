# SPDX-License-Identifier: GPL-3.0-or-later
"""Bound the pinned core's raster queues and padded collision scratch."""
def apply_raster_guards(output):
    # Visible160 + widest copy offset64 + largest expanded player32.
    for name in ('collisio.c', 'collisio.h'):
        path = output / name
        text = path.read_text()
        assert 'colvect[230]' in text
        path.write_text(text.replace('colvect[230]', 'colvect[256]'))
    path = output / 'vmachine.c'
    text = path.read_text()
    old = 'void do_unified_change(int type, int val)\n{'
    assert old in text
    text = text.replace(old, old + '\n\tif ((unsigned)unified_count >= 80) return;')
    old = 'int plc=pl_change_count[i];'
    assert old in text
    text = text.replace(old, 'int plc;\n\tif ((unsigned)i >= 2) return;\n\tplc=pl_change_count[i];')
    text = text.replace('if (plc < 81)', 'if ((unsigned)plc < 80)')
    old = 'int pfc=pf_change_count[i];'
    assert old in text
    text = text.replace(old, 'int pfc;\n\tif (i != 0) return;\n\tpfc=pf_change_count[i];')
    text = text.replace('if (pfc < 81)', 'if ((unsigned)pfc < 80)')
    path.write_text(text)
