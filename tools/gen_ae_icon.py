#!/usr/bin/env python3
"""Generate QA icon byte arrays from ae.png.

Normal  : ae.png as-is
Hover   : 30% brighter RGB, content scaled up 10% (cropped to same canvas)
"""

import io
import os
import sys
from PIL import Image, ImageEnhance

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def brighten(img, factor):
    r, g, b, a = img.split()
    rgb = Image.merge("RGB", (r, g, b))
    rgb = ImageEnhance.Brightness(rgb).enhance(factor)
    r, g, b = rgb.split()
    return Image.merge("RGBA", (r, g, b, a))


def scale_up(img, factor):
    """Scale content up by factor, center-crop back to original canvas size."""
    w, h = img.size
    new_w = int(round(w * factor))
    new_h = int(round(h * factor))
    scaled = img.resize((new_w, new_h), Image.LANCZOS)
    left = (new_w - w) // 2
    top  = (new_h - h) // 2
    return scaled.crop((left, top, left + w, top + h))


def to_c_array(img, var_name):
    buf = io.BytesIO()
    img.save(buf, format="PNG", optimize=True)
    data = buf.getvalue()
    lines = [f"static const unsigned char {var_name}[] = {{"]
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        lines.append("    " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
    lines.append("};")
    lines.append(f"static const unsigned int {var_name}_size = {len(data)};")
    return "\n".join(lines)


if __name__ == "__main__":
    src = os.path.join(ROOT, "ae.png")
    if not os.path.exists(src):
        print(f"ERROR: {src} not found", file=sys.stderr)
        sys.exit(1)

    base = Image.open(src).convert("RGBA")

    normal = base.copy()
    hover  = scale_up(brighten(base.copy(), 1.2), 1.1)

    print("// --- Normal icon ---")
    print(to_c_array(normal, "ICON_NORMAL"))
    print()
    print("// --- Hover icon ---")
    print(to_c_array(hover, "ICON_HOVER"))
