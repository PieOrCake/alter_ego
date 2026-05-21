#!/usr/bin/env python3
"""
Fetch GW2 wiki loading-screen images for each raid wing & strike mission,
crop them into a wide banner ratio, and emit a C++ header containing the
JPEG bytes as static arrays + a lookup table.

Output: src/EmbeddedBanners.h
"""
from __future__ import annotations

import io
import os
import sys
import textwrap
import time
from pathlib import Path

import requests
from PIL import Image

WIKI = "https://wiki.guildwars2.com/api.php"
USER_AGENT = "AlterEgo-banner-prebake/1.0 (https://github.com/PieOrCake/alter_ego)"
HEADERS = {"User-Agent": USER_AGENT}

# Final banner dimensions in pixels.
BANNER_W = 600
BANNER_H = 80
JPEG_QUALITY = 78

ROOT = Path(__file__).resolve().parent.parent
OUT_HEADER = ROOT / "src" / "EmbeddedBanners.h"
CACHE = ROOT / "tools" / ".banner_cache"
CACHE.mkdir(exist_ok=True)

# (key_for_lookup, wiki_page_title)
# Keys are what the C++ side looks up: "wing:9128" or "strike:Cold War"
WINGS = [
    ("wing:9128", "Spirit Vale"),
    ("wing:9147", "Salvation Pass"),
    ("wing:9182", "Stronghold of the Faithful"),
    ("wing:9144", "Bastion of the Penitent"),
    ("wing:9111", "Hall of Chains"),
    ("wing:9120", "Mythwright Gambit"),
    ("wing:9156", "The Key of Ahdashim"),
    ("wing:9181", "Mount Balrior"),
]

# Strike pages. Each entry is a list of candidates — we try them in order until
# one yields an image. A candidate can be a wiki page title, or a "File:..." title
# to fetch that image directly.
STRIKES = [
    ("strike:Shiverpeaks Pass",     ["Strike Mission: Shiverpeaks Pass", "A Race to Arms", "Boneskinner"]),
    ("strike:Voice of the Fallen and Claw of the Fallen", ["Strike Mission: Voice of the Fallen and Claw of the Fallen", "The Invitation"]),
    ("strike:Fraenir of Jormag",    ["Strike Mission: Fraenir of Jormag", "The Invitation"]),
    ("strike:Boneskinner",          ["File:The Invitation loading screen.jpg"]),
    ("strike:Whisper of Jormag",    ["Strike Mission: Whisper of Jormag"]),
    ("strike:Forging Steel",        ["Strike Mission: Forging Steel"]),
    ("strike:Cold War",             ["Strike Mission: Cold War"]),
    ("strike:Aetherblade Hideout",  ["Strike Mission: Aetherblade Hideout"]),
    ("strike:Xunlai Jade Junkyard", ["Strike Mission: Xunlai Jade Junkyard"]),
    ("strike:Kaineng Overlook",     ["Strike Mission: Kaineng Overlook"]),
    ("strike:Harvest Temple",       ["Strike Mission: Harvest Temple"]),
    ("strike:Old Lion's Court",     ["Strike Mission: Old Lion's Court"]),
    ("strike:Cosmic Observatory",   ["File:Mother of Stars.png"]),
    ("strike:Temple of Febe",       ["File:Hell Breaks Loose.png"]),
    ("strike:Guardian's Glade",     ["File:Shipwreck Strand loading screen.png"]),
]


def find_loading_screen_file(page_title: str) -> str | None:
    """Query the wiki for images on a page; return the File: title of the first
    loading-screen-looking image we find — preferring one whose name contains
    a normalized form of the strike/wing name."""
    r = requests.get(WIKI, params={
        "action": "query",
        "prop": "images",
        "titles": page_title,
        "imlimit": "max",
        "redirects": "1",
        "format": "json",
    }, headers=HEADERS, timeout=20)
    r.raise_for_status()
    data = r.json()
    pages = data.get("query", {}).get("pages", {})

    # Build name tokens to match against image titles (drop "Strike Mission:" prefix)
    name_for_match = page_title
    if name_for_match.lower().startswith("strike mission:"):
        name_for_match = name_for_match.split(":", 1)[1].strip()
    name_tokens = [w for w in name_for_match.lower().split() if len(w) >= 4 and w not in ("the", "and", "of", "for")]

    candidates: list[tuple[int, str]] = []  # (score, title)
    for _pid, p in pages.items():
        for img in p.get("images", []):
            title = img["title"]
            lower = title.lower()
            if "loading screen" not in lower and "loadingscreen" not in lower and "loading_screen" not in lower:
                continue
            # Score: higher if image title shares more tokens with the entity name
            score = sum(1 for tok in name_tokens if tok in lower)
            candidates.append((score, title))

    if candidates:
        candidates.sort(reverse=True)  # highest score first
        return candidates[0][1]
    return None


def get_image_url(file_title: str) -> str | None:
    r = requests.get(WIKI, params={
        "action": "query",
        "prop": "imageinfo",
        "iiprop": "url",
        "titles": file_title,
        "format": "json",
    }, headers=HEADERS, timeout=20)
    r.raise_for_status()
    data = r.json()
    pages = data.get("query", {}).get("pages", {})
    for _pid, p in pages.items():
        infos = p.get("imageinfo", [])
        if infos:
            return infos[0]["url"]
    return None


def fetch_image(url: str) -> bytes:
    r = requests.get(url, headers=HEADERS, timeout=30)
    r.raise_for_status()
    return r.content


def crop_to_banner(raw: bytes) -> bytes:
    """Crop an image to BANNER_W x BANNER_H aspect, scaled to BANNER_W wide."""
    img = Image.open(io.BytesIO(raw)).convert("RGB")

    target_ratio = BANNER_W / BANNER_H
    src_w, src_h = img.size
    src_ratio = src_w / src_h

    if src_ratio > target_ratio:
        # Source is wider than target — crop horizontally (centered)
        new_w = int(src_h * target_ratio)
        x0 = (src_w - new_w) // 2
        img = img.crop((x0, 0, x0 + new_w, src_h))
    else:
        # Source is taller than target — crop vertically (centered, prefer slightly above center)
        new_h = int(src_w / target_ratio)
        y0 = max(0, (src_h - new_h) // 3)  # bias toward upper third so we keep faces
        img = img.crop((0, y0, src_w, y0 + new_h))

    img = img.resize((BANNER_W, BANNER_H), Image.LANCZOS)
    out = io.BytesIO()
    img.save(out, format="JPEG", quality=JPEG_QUALITY, optimize=True)
    return out.getvalue()


def safe_ident(s: str) -> str:
    """Turn a key like 'strike:Old Lion\\'s Court' into a valid C identifier."""
    return "".join(c if c.isalnum() else "_" for c in s).strip("_").lower()


def cache_path(key: str) -> Path:
    return CACHE / (safe_ident(key) + ".jpg")


def fetch_one(key: str, page_or_pages) -> bytes | None:
    cp = cache_path(key)
    if cp.exists():
        print(f"  cached  {key:50s} -> {cp.name}")
        return cp.read_bytes()

    # Allow a single page string or a list of candidate pages.
    pages = page_or_pages if isinstance(page_or_pages, list) else [page_or_pages]
    file_title = None
    chosen_page = None
    for pg in pages:
        # Direct File: override — skip discovery, just use this image.
        if pg.startswith("File:"):
            print(f"  fetching {key:50s} (direct: {pg})")
            file_title = pg
            chosen_page = pg
            break
        print(f"  fetching {key:50s} ({pg})")
        ft = find_loading_screen_file(pg)
        if ft:
            file_title = ft
            chosen_page = pg
            break
        else:
            print(f"    ! no loading screen image found on '{pg}'")
    if not file_title:
        return None
    url = get_image_url(file_title)
    if not url:
        print(f"    ! no URL for {file_title}")
        return None
    print(f"    {file_title}  ({url})")
    raw = fetch_image(url)
    cropped = crop_to_banner(raw)
    cp.write_bytes(cropped)
    _ = chosen_page  # for debug clarity
    time.sleep(0.4)  # be polite to the wiki
    return cropped


def emit_array(name: str, data: bytes) -> str:
    rows = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        rows.append("    " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
    body = "\n".join(rows)
    return (
        f"static const unsigned char {name}[] = {{\n"
        f"{body}\n"
        f"}};\n"
        f"static const unsigned int {name}_len = {len(data)};\n"
    )


def main() -> int:
    items = WINGS + STRIKES
    blobs: list[tuple[str, str, bytes]] = []  # (key, c_ident, bytes)

    for key, page in items:
        data = fetch_one(key, page)
        if data is None:
            continue
        ident = "banner_" + safe_ident(key)
        blobs.append((key, ident, data))

    if not blobs:
        print("No banners fetched; aborting.", file=sys.stderr)
        return 1

    parts = [textwrap.dedent("""\
        // Auto-generated by tools/fetch_banners.py.  Do not edit by hand.
        // Re-run the script when new strikes / raid wings are added.
        #pragma once

        #include <cstddef>
        #include <string>

        namespace AlterEgo {
        namespace EmbeddedBanners {

        """)]

    for _key, ident, data in blobs:
        parts.append(emit_array(ident, data))
        parts.append("\n")

    # Lookup table
    parts.append("struct BannerEntry { const char* key; const unsigned char* data; unsigned int len; };\n")
    parts.append("static const BannerEntry kBanners[] = {\n")
    for key, ident, _data in blobs:
        esc_key = key.replace('\\', '\\\\').replace('"', '\\"')
        parts.append(f'    {{ "{esc_key}", {ident}, {ident}_len }},\n')
    parts.append("};\n")
    parts.append("static const unsigned int kBannerCount = sizeof(kBanners) / sizeof(kBanners[0]);\n\n")

    parts.append(textwrap.dedent("""\
        inline const unsigned char* FindBanner(const std::string& key, unsigned int& outLen) {
            for (unsigned int i = 0; i < kBannerCount; i++) {
                if (key == kBanners[i].key) {
                    outLen = kBanners[i].len;
                    return kBanners[i].data;
                }
            }
            outLen = 0;
            return nullptr;
        }

        } // namespace EmbeddedBanners
        } // namespace AlterEgo
        """))

    OUT_HEADER.write_text("".join(parts))
    total_bytes = sum(len(b) for _k, _i, b in blobs)
    print()
    print(f"Wrote {OUT_HEADER.relative_to(ROOT)}")
    print(f"{len(blobs)} banners, total {total_bytes/1024:.1f} KB embedded JPEG.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
