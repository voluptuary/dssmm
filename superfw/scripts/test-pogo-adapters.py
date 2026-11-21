#!/usr/bin/env python3
"""Check portable patch signatures and halfword LZ output against independent data."""
import argparse,ctypes as C,struct,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--pack',type=Path);args=p.parse_args()
def decode(data):
    total=int.from_bytes(data[1:4],'little');out=bytearray();ip=4
    while len(out)<total:
        flags=data[ip];ip+=1
        for bit in range(8):
            if len(out)==total:break
            if flags&(128>>bit):
                a,b=data[ip:ip+2];ip+=2
                for _ in range((a>>4)+3):out.append(out[-(((a&15)<<8)+b+1)])
            else:out.append(data[ip]);ip+=1
    return bytes(out)
with tempfile.TemporaryDirectory() as tmp:
    lib=Path(tmp)/'pogo.so'
    subprocess.run(['gcc','-shared','-fPIC','-O2','-Wall','-Wextra','-Werror','-I'+str(root/'src'),str(root/'src/pogo_plugin.c'),'-o',str(lib)],check=True)
    dll=C.CDLL(str(lib));dll.pogo_adapt_image.argtypes=[C.c_void_p,C.c_uint,C.c_uint]
    dll.pogo_unpack16.argtypes=[C.c_void_p,C.c_uint,C.c_void_p]
    def adapt(data,target=0x09a00000):
        buf=(C.c_uint32*((len(data)+3)//4+2))();C.memmove(buf,data,len(data))
        count=dll.pogo_adapt_image(buf,len(data),target)
        return count,bytes(buf)[:len(data)]
    # Known complete reset body from SMSAdvance 2.5; mutations must be rejected.
    words=[0xe3a01301,0xe3a00000,0xe1c10bba,0xe1c10cb6,0xe1c10db2,0xe1c10dbe,0xe5810208,0xebffffd6,0xe3a00000,0xe59f1090,0xe1c100b0,0xebfffff0,0xe3a0000c,0xef010000,0xef000000]
    data=struct.pack('<15I',*words)
    count,fixed=adapt(data);assert count==1 and fixed[:8]==struct.pack('<II',0xe51ff004,0x09a00000) and fixed[8:]==data[8:]
    assert adapt(data,0)==(0,data)
    for i in range(len(words)):
        bad=bytearray(data);bad[i*4]^=1;assert adapt(bytes(bad))==(0,bytes(bad))
    for n in range(len(data)):assert adapt(data[:n])[0]==0
    def unpack(data):
        source=(C.c_uint8*len(data)).from_buffer_copy(data)
        out=(C.c_uint16*(0x40000//2+8))(*([0xa55a]*(0x40000//2+8)))
        n=dll.pogo_unpack16(source,len(data),out)
        if n:
            expected=decode(data);assert n==len(expected) and bytes(out)[:n]==expected
            end=(n+3)&~3;assert bytes(out)[n:end]==bytes(end-n)
            assert bytes(out)[end:end+8]==b'\x5a\xa5'*4
        return n
    assert unpack(bytes([0x10,7,0,0,0x40,0x42,0x30,0]))==7 # B then six overlapping B copies.
    assert not unpack(bytes([0x10,7,0,0,0x80,0x30,0]))
    candidates=list((root/'emu').glob('*scptch.gba.bin'))
    if args.pack:candidates+=list(args.pack.glob('*.mb'))+list(args.pack.glob('*.mbz'))
    for path in candidates:
        raw=path.read_bytes();decoded=decode(raw) if path.suffix=='.mbz' else raw
        if path.suffix=='.mbz':assert unpack(raw)==len(decoded)
        count,fixed=adapt(decoded)
        if path.name in ('sms.mb','gg.mb','sg.mb','pce.mb'):assert count==1,(path,count)
        print(path.name,':',len(decoded),'decoded bytes;',count,'recognized return hook(s)')
print('Exact return signatures reject mutations; overlapping/truncated LZ and real-pack adaptation passed.')
