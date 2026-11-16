#!/usr/bin/env python3
import sys,re
from pathlib import Path
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'scripts'))
from firmware_limits import LIMITS,check_size
make=(root/'Makefile').read_text()
for board,limit in LIMITS.items():
    branch=make.split(f'($(BOARD),{board})',1)[1].split('else',1)[0]
    assert int(re.search(r'MAXFSIZE\s*=\s*(\d+)',branch)[1])*1024==limit
    assert check_size(board,limit)['firmware_headroom_bytes']==0
    for invalid in (0,-1,limit+512):
        try:check_size(board,invalid)
        except ValueError:pass
        else:raise AssertionError(f'{board}: invalid image accepted')
try:check_size('other',512)
except ValueError:pass
else:raise AssertionError('Unknown hardware accepted')
print('All three firmware budgets match board build limits; oversized images rejected.')
