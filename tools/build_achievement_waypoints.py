#!/usr/bin/env python3
"""Build achievement_waypoints.json by cross-referencing achievement bits with GW2 API POI data.

This script:
1. Fetches all POIs (waypoints, landmarks, vistas) from the GW2 continents API
2. Fetches achievements that have location-based bits (Text type)
3. Matches bit text to POI names, preferring same-map matches
4. Outputs matches for manual review before committing

Usage:
    python3 tools/build_achievement_waypoints.py [--auto] [--ach-id ID]

    --auto      Write matches directly to data/achievement_waypoints.json
    --ach-id    Only process a specific achievement ID
    --merge     Merge with existing data file
    --map NAME  Hint: restrict fuzzy matches to this map name
"""
import urllib.request
import json
import sys
import os
import argparse
import re
from difflib import SequenceMatcher

API_BASE = "https://api.guildwars2.com/v2"
OUTPUT_PATH = os.path.join(os.path.dirname(os.path.dirname(__file__)), "data", "achievement_waypoints.json")

# Tyria (1) and Mists (2) continent IDs
CONTINENT_IDS = [1, 2]

# Suffixes stripped when comparing POI names to bit text
POI_SUFFIXES = [" waypoint", " vista", " point of interest", " poi"]


def api_get(url):
    """Fetch JSON from GW2 API."""
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "AlterEgo/1.0"})
        with urllib.request.urlopen(req, timeout=30) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except Exception as e:
        print(f"  Error fetching {url}: {e}", file=sys.stderr)
        return None


def fetch_all_pois():
    """Fetch all POIs (waypoints + landmarks + vistas) from all continents/floors."""
    pois = []       # list of {name, chat_link, map, type, coord}
    by_map = {}     # map_name_lower -> [poi, ...]
    seen = set()    # dedupe by chat_link

    for cont_id in CONTINENT_IDS:
        floors = api_get(f"{API_BASE}/continents/{cont_id}/floors")
        if not floors:
            continue

        for floor_id in floors[:3]:  # First few floors cover most content
            floor_data = api_get(f"{API_BASE}/continents/{cont_id}/floors/{floor_id}")
            if not floor_data or "regions" not in floor_data:
                continue

            for region in floor_data["regions"].values():
                for game_map in region.get("maps", {}).values():
                    map_name = game_map.get("name", "")
                    for poi in game_map.get("points_of_interest", {}).values():
                        poi_type = poi.get("type", "")
                        if poi_type not in ("waypoint", "landmark", "vista"):
                            continue
                        name = poi.get("name", "")
                        chat_link = poi.get("chat_link", "")
                        if not chat_link or chat_link in seen:
                            continue
                        seen.add(chat_link)
                        entry = {
                            "name": name or f"Unnamed {poi_type}",
                            "chat_link": chat_link,
                            "map": map_name,
                            "type": poi_type,
                            "coord": poi.get("coord", []),
                        }
                        pois.append(entry)
                        by_map.setdefault(map_name.lower(), []).append(entry)

            sys.stdout.write(f"\r  Continent {cont_id}, floor {floor_id}: {len(pois)} POIs")
            sys.stdout.flush()

    print(f"\n  Total: {len(pois)} POIs across {len(by_map)} maps")
    return pois, by_map


def strip_poi_suffix(name):
    """Remove common POI type suffixes for comparison."""
    lower = name.lower().strip()
    for suffix in POI_SUFFIXES:
        if lower.endswith(suffix):
            lower = lower[:-len(suffix)].strip()
    return lower


def find_poi_match(bit_text, pois, by_map, map_hint=None):
    """Match bit text to a POI. Prefer exact matches, then same-map, then best fuzzy.

    Returns (poi_entry, confidence) or (None, 0).
    Confidence: 1.0 = exact, 0.9+ = very close same-map, 0.7+ = fuzzy same-map.
    """
    bit_lower = bit_text.lower().strip()
    bit_stripped = strip_poi_suffix(bit_lower)

    # Pass 1: Exact name match (case-insensitive)
    for poi in pois:
        poi_lower = poi["name"].lower()
        if bit_lower == poi_lower or bit_stripped == poi_lower:
            return poi, 1.0
        poi_stripped = strip_poi_suffix(poi_lower)
        if bit_stripped == poi_stripped:
            return poi, 1.0

    # Pass 2: Exact substring — bit text appears in POI name or vice versa (same map preferred)
    candidates = []
    search_pois = by_map.get(map_hint.lower(), []) if map_hint else pois
    for poi in search_pois:
        poi_stripped = strip_poi_suffix(poi["name"].lower())
        if bit_stripped in poi_stripped or poi_stripped in bit_stripped:
            candidates.append((poi, 0.95))

    if candidates:
        # Prefer waypoints over landmarks
        candidates.sort(key=lambda x: (0 if x[0]["type"] == "waypoint" else 1))
        return candidates[0]

    # Pass 3: Fuzzy match — only within the hinted map if provided
    if map_hint:
        search_pois = by_map.get(map_hint.lower(), [])
    else:
        search_pois = pois

    best_match = None
    best_ratio = 0.7  # minimum threshold
    for poi in search_pois:
        poi_stripped = strip_poi_suffix(poi["name"].lower())
        ratio = SequenceMatcher(None, bit_stripped, poi_stripped).ratio()
        if ratio > best_ratio:
            best_ratio = ratio
            best_match = poi
        # Also try matching against just the first word(s) for compound names
        if " " in bit_stripped:
            first_part = bit_stripped.split()[0]
            if len(first_part) > 3:
                ratio2 = SequenceMatcher(None, first_part, poi_stripped).ratio()
                if ratio2 > best_ratio:
                    best_ratio = ratio2
                    best_match = poi

    if best_match:
        return best_match, best_ratio

    return None, 0


def guess_map_from_achievement(ach):
    """Try to extract a map name hint from the achievement name/description."""
    # Common patterns: "Lighting the Echovald Wilds", "Kessex Hills Explorer"
    name = ach.get("name", "")
    desc = ach.get("description", "")

    # Known map name patterns in achievement names
    # The achievement name often contains "the <MapName>" or "<MapName> <Suffix>"
    map_patterns = [
        r"(?:in|of|the)\s+(.+?)(?:\s*$)",
    ]

    return None  # Don't guess — let --map flag handle it


def process_achievement(ach_id, pois, by_map, map_hint=None):
    """Process a single achievement and return POI matches."""
    ach = api_get(f"{API_BASE}/achievements/{ach_id}")
    if not ach or "bits" not in ach:
        return None, None

    matches = {}
    unmatched = []
    for i, bit in enumerate(ach.get("bits", [])):
        if bit.get("type") != "Text":
            continue
        text = bit.get("text", "")
        if not text:
            continue

        poi, confidence = find_poi_match(text, pois, by_map, map_hint)
        if poi:
            matches[str(i)] = poi["chat_link"]
            conf_str = f"{confidence:.0%}"
            marker = "  " if confidence >= 0.95 else " ~"
            print(f"  {marker} Bit {i}: '{text}' -> {poi['name']} ({poi['chat_link']}) "
                  f"[{poi['type']}] in {poi['map']} ({conf_str})")
        else:
            unmatched.append((i, text))

    if unmatched:
        print(f"\n  Unmatched bits ({len(unmatched)}):")
        for i, text in unmatched:
            print(f"     Bit {i}: '{text}'")

    return matches if matches else None, ach


def main():
    parser = argparse.ArgumentParser(description="Build achievement waypoint/POI mappings")
    parser.add_argument("--auto", action="store_true", help="Write output automatically")
    parser.add_argument("--ach-id", type=int, help="Process a specific achievement ID")
    parser.add_argument("--merge", action="store_true", help="Merge with existing data")
    parser.add_argument("--map", type=str, help="Restrict fuzzy matches to this map name")
    args = parser.parse_args()

    # Load existing data if merging
    existing = {}
    if args.merge and os.path.exists(OUTPUT_PATH):
        with open(OUTPUT_PATH) as f:
            existing = json.load(f)
        print(f"Loaded {len(existing)} existing entries")

    # Fetch POIs
    print("Fetching POIs from GW2 API...")
    pois, by_map = fetch_all_pois()

    if args.map:
        # Show available maps matching the hint
        matching_maps = [m for m in by_map if args.map.lower() in m]
        if matching_maps:
            print(f"\n  Maps matching '{args.map}': {', '.join(matching_maps)}")
            poi_count = sum(len(by_map[m]) for m in matching_maps)
            print(f"  {poi_count} POIs in matched maps")

    if args.ach_id:
        print(f"\nProcessing achievement {args.ach_id}...")
        matches, ach = process_achievement(args.ach_id, pois, by_map, args.map)
        if ach:
            print(f"\n  {ach.get('name', 'Unknown')}")
        if matches:
            existing[str(args.ach_id)] = matches
            print(f"  Found {len(matches)} POI matches")
        else:
            print("  No POI matches found")
    else:
        print("\nNo --ach-id specified. Use --ach-id to process specific achievements.")
        print("Example: python3 tools/build_achievement_waypoints.py --ach-id 6219 --map 'Echovald Wilds'")
        if by_map:
            print(f"\nAvailable maps ({len(by_map)}):")
            for m in sorted(by_map.keys()):
                print(f"  {m} ({len(by_map[m])} POIs)")
        return

    if args.auto and existing:
        os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
        with open(OUTPUT_PATH, "w") as f:
            json.dump(existing, f, indent=2, sort_keys=True)
        print(f"\nSaved to {OUTPUT_PATH}")
    elif existing:
        print("\nRun with --auto to save, or copy the JSON manually:")
        print(json.dumps(existing, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
