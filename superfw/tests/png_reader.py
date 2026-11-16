"""Small standard-library RGB PNG reader shared by mGBA screenshot checks."""
from pathlib import Path
import struct,zlib

def read_rgb_png(path):
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise SystemExit("mGBA screenshot is not a PNG")
    offset, chunks, header = 8, [], None
    while offset < len(data):
        size = struct.unpack_from(">I", data, offset)[0]
        kind = data[offset + 4:offset + 8]
        payload = data[offset + 8:offset + 8 + size]
        offset += size + 12
        if kind == b"IHDR":
            header = struct.unpack(">IIBBBBB", payload)
        elif kind == b"IDAT":
            chunks.append(payload)
        elif kind == b"IEND":
            break
    if not header:
        raise SystemExit("PNG has no IHDR")
    width, height, depth, color_type, compression, filtering, interlace = header
    if (depth, color_type, interlace) != (8, 2, 0):
        raise SystemExit(f"Unexpected screenshot format: {header}; expected 8-bit RGB, non-interlaced")
    raw = zlib.decompress(b"".join(chunks))
    stride, bpp = width * 3, 3
    if len(raw) != height * (stride + 1):
        raise SystemExit("PNG pixel data has an unexpected length")
    rows, previous = [], bytearray(stride)
    pos = 0
    for _ in range(height):
        method = raw[pos]
        row = bytearray(raw[pos + 1:pos + 1 + stride])
        pos += stride + 1
        for i in range(stride):
            left = row[i - bpp] if i >= bpp else 0
            above = previous[i]
            upper_left = previous[i - bpp] if i >= bpp else 0
            if method == 1:
                row[i] = (row[i] + left) & 255
            elif method == 2:
                row[i] = (row[i] + above) & 255
            elif method == 3:
                row[i] = (row[i] + ((left + above) // 2)) & 255
            elif method == 4:
                estimate = left + above - upper_left
                distances = (abs(estimate - left), abs(estimate - above), abs(estimate - upper_left))
                predictor = (left, above, upper_left)[distances.index(min(distances))]
                row[i] = (row[i] + predictor) & 255
            elif method != 0:
                raise SystemExit(f"Unsupported PNG filter {method}")
        rows.append(row)
        previous = row
    return width,height,rows

def artwork_pixel_count(rows):
    """Count fixture cube colors without mistaking gray DS panels for artwork."""
    cube={0,49,99,156,206,255}
    return sum(set(row[i:i+3])<=cube and max(row[i:i+3])-min(row[i:i+3])>=32
               for row in rows for i in range(0,len(row),3))
