#!/usr/bin/env python3
"""
Build data/hero_challenges.json from the GW2 continents API.

Generates a mapping of hero challenge IDs to their map/expansion info.
The character endpoint /v2/characters/:name/heropoints returns completed IDs.
Diffing against this file gives incomplete hero challenges.

Coverage:
- Core Tyria, HoT, LWS2, PoF: full (IDs from continents API)
- EoD, JW: coordinates only (no IDs in API — need manual curation)
- LWS3, LWS4, IBS, SotO: not in continents API skill_challenges at all

Usage: python3 tools/build_hero_challenges.py
"""

import json
import urllib.request
import sys
from collections import defaultdict

# Map IDs to expansion grouping (maps not listed here default to their region)
EXPANSION_BY_MAP_ID = {
    # HoT
    1041: "Heart of Thorns", 1043: "Heart of Thorns", 1045: "Heart of Thorns", 1052: "Heart of Thorns",
    # LWS2
    988: "Living World Season 2", 1015: "Living World Season 2",
    # PoF
    1210: "Path of Fire", 1211: "Path of Fire", 1226: "Path of Fire", 1228: "Path of Fire", 1248: "Path of Fire",
    # EoD
    1422: "End of Dragons", 1428: "End of Dragons", 1438: "End of Dragons", 1442: "End of Dragons", 1452: "End of Dragons",
    # JW
    1550: "Janthir Wilds", 1554: "Janthir Wilds", 1564: "Janthir Wilds", 1593: "Janthir Wilds", 1595: "Janthir Wilds",
}

CORE_REGIONS = {"Shiverpeak Mountains", "Ascalon", "Ruins of Orr", "Kryta", "Tarnished Coast", "Steamspur Mountains"}

def fetch_json(url):
    return json.loads(urllib.request.urlopen(url).read())

def get_expansion(map_id, region_name):
    if map_id in EXPANSION_BY_MAP_ID:
        return EXPANSION_BY_MAP_ID[map_id]
    if region_name in CORE_REGIONS:
        return "Core"
    return region_name  # fallback

def scan_floor(continent, floor):
    url = f"https://api.guildwars2.com/v2/continents/{continent}/floors/{floor}"
    try:
        data = fetch_json(url)
    except Exception as e:
        print(f"  Error fetching floor {floor}: {e}", file=sys.stderr)
        return []

    results = []
    for rid, region in data.get("regions", {}).items():
        for mid, m in region.get("maps", {}).items():
            map_id = int(mid)
            for sc in m.get("skill_challenges", []):
                hc_id = sc.get("id", "")
                if not hc_id:
                    continue  # skip challenges without IDs
                results.append({
                    "id": hc_id,
                    "map_id": map_id,
                    "map_name": m["name"],
                    "expansion": get_expansion(map_id, region["name"]),
                    "coord": sc["coord"],
                })
    return results

def main():
    all_hc = {}

    # Floor 1: Core, HoT, LWS2, EoD (EoD has no IDs though)
    print("Scanning continent 1, floor 1...")
    for hc in scan_floor(1, 1):
        all_hc[hc["id"]] = hc

    # Floor 49: PoF
    print("Scanning continent 1, floor 49...")
    for hc in scan_floor(1, 49):
        if hc["id"] not in all_hc:
            all_hc[hc["id"]] = hc

    # Organize by expansion -> map
    by_expansion = defaultdict(lambda: defaultdict(list))
    for hc in sorted(all_hc.values(), key=lambda x: (x["expansion"], x["map_name"], x["id"])):
        by_expansion[hc["expansion"]][hc["map_name"]].append(hc["id"])

    # Print summary
    total = len(all_hc)
    print(f"\nTotal hero challenges with IDs: {total}")
    for exp in sorted(by_expansion.keys()):
        maps = by_expansion[exp]
        count = sum(len(ids) for ids in maps.values())
        print(f"  {exp}: {count} ({len(maps)} maps)")
        for map_name in sorted(maps.keys()):
            print(f"    {map_name}: {len(maps[map_name])}")

    # Build output: { "hero_challenges": { id: { map_name, map_id, expansion } }, "maps": { expansion: [map_name, ...] } }
    output = {}
    for hc in all_hc.values():
        output[hc["id"]] = {
            "map_id": hc["map_id"],
            "map_name": hc["map_name"],
            "expansion": hc["expansion"],
        }

    out_path = "data/hero_challenges.json"
    with open(out_path, "w") as f:
        json.dump(output, f, separators=(",", ":"), sort_keys=True)
    print(f"\nWrote {out_path} ({len(output)} entries)")

if __name__ == "__main__":
    main()
