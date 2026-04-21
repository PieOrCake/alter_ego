#!/usr/bin/env python3
"""Generate a simplified 32x32 Janus (dual-face) icon for Nexus Quick Access bar.

Produces two PNGs:
  - janus_normal.png  (beige fill, dark outline — idle state)
  - janus_hover.png   (brighter fill — hover state)

Then prints C byte arrays ready to embed in dllmain.cpp.
"""

from PIL import Image, ImageDraw

W, H = 32, 32

# Colors matching GW2 Nexus QA bar style
BEIGE_NORMAL = (180, 165, 135, 255)   # muted beige for idle
BEIGE_HOVER  = (220, 205, 175, 255)   # brighter beige for hover
OUTLINE      = (30, 25, 20, 255)      # near-black outline
TRANSPARENT  = (0, 0, 0, 0)


def draw_janus(draw, fill, outline):
    """Draw a simplified Janus icon: two opposing profile silhouettes on a coin."""

    # Coin circle border (outer ring)
    draw.ellipse([1, 1, 30, 30], fill=None, outline=outline, width=2)

    # Left-facing profile (left half)
    # Forehead -> nose -> lips -> chin
    left_profile = [
        (16, 5),    # top of head center
        (12, 5),    # forehead left
        (10, 7),    # brow
        (8, 8),     # brow ridge
        (7, 10),    # upper nose bridge
        (5, 13),    # nose tip
        (7, 14),    # under nose
        (7, 15),    # upper lip
        (6, 16),    # lip point
        (7, 17),    # lower lip
        (8, 18),    # chin start
        (8, 21),    # chin
        (10, 23),   # jaw
        (13, 25),   # neck
        (16, 26),   # center bottom
    ]

    # Right-facing profile (mirror of left)
    right_profile = [
        (16, 5),    # top of head center
        (20, 5),    # forehead right
        (22, 7),    # brow
        (24, 8),    # brow ridge
        (25, 10),   # upper nose bridge
        (27, 13),   # nose tip
        (25, 14),   # under nose
        (25, 15),   # upper lip
        (26, 16),   # lip point
        (25, 17),   # lower lip
        (24, 18),   # chin start
        (24, 21),   # chin
        (22, 23),   # jaw
        (19, 25),   # neck
        (16, 26),   # center bottom
    ]

    # Fill the left face silhouette
    left_fill = left_profile + [(16, 26), (16, 5)]
    draw.polygon(left_fill, fill=fill)

    # Fill the right face silhouette
    right_fill = right_profile + [(16, 26), (16, 5)]
    draw.polygon(right_fill, fill=fill)

    # Draw profile outlines
    draw.line(left_profile, fill=outline, width=1)
    draw.line(right_profile, fill=outline, width=1)

    # Center dividing line (subtle)
    draw.line([(16, 6), (16, 25)], fill=outline, width=1)


def generate_icon(filename, fill, outline):
    img = Image.new("RGBA", (W, H), TRANSPARENT)
    draw = ImageDraw.Draw(img)
    draw_janus(draw, fill, outline)
    img.save(filename)
    return img


def png_to_c_array(filename, var_name):
    with open(filename, "rb") as f:
        data = f.read()
    lines = []
    lines.append(f"static const unsigned char {var_name}[] = {{")
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_str = ", ".join(f"0x{b:02x}" for b in chunk)
        lines.append(f"    {hex_str},")
    lines.append("};")
    lines.append(f"static const unsigned int {var_name}_size = {len(data)};")
    return "\n".join(lines)


if __name__ == "__main__":
    import os
    out_dir = os.path.dirname(os.path.abspath(__file__))

    normal_path = os.path.join(out_dir, "janus_normal.png")
    hover_path = os.path.join(out_dir, "janus_hover.png")

    generate_icon(normal_path, BEIGE_NORMAL, OUTLINE)
    generate_icon(hover_path, BEIGE_HOVER, OUTLINE)

    print(f"Generated: {normal_path}")
    print(f"Generated: {hover_path}")
    print()
    print("// --- Normal icon ---")
    print(png_to_c_array(normal_path, "ICON_NORMAL"))
    print()
    print("// --- Hover icon ---")
    print(png_to_c_array(hover_path, "ICON_HOVER"))
