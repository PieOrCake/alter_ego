#!/usr/bin/env python3
"""Fetch all GW2 achievement names from the API and output data/achievement_names.json.

This generates the name index used by the Alter Ego addon for achievement search.
Run periodically after major GW2 content patches to pick up new achievements.

Usage:
    python3 tools/fetch_achievement_names.py
"""
import urllib.request
import json
import sys
import os

API_BASE = "https://api.guildwars2.com/v2"
BATCH_SIZE = 200
OUTPUT_PATH = os.path.join(os.path.dirname(os.path.dirname(__file__)), "data", "achievement_names.json")


def api_get(url):
    """Fetch JSON from GW2 API."""
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "AlterEgo/1.0"})
        with urllib.request.urlopen(req, timeout=30) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except Exception as e:
        print(f"  Error fetching {url}: {e}", file=sys.stderr)
        return None


def main():
    # Step 1: Get all achievement IDs
    print("Fetching achievement ID list...")
    all_ids = api_get(f"{API_BASE}/achievements")
    if not all_ids or not isinstance(all_ids, list):
        print("Failed to fetch achievement IDs", file=sys.stderr)
        sys.exit(1)
    print(f"  Found {len(all_ids)} achievement IDs")

    # Step 2: Fetch names in batches
    name_index = {}
    total = len(all_ids)
    for start in range(0, total, BATCH_SIZE):
        end = min(start + BATCH_SIZE, total)
        batch_ids = all_ids[start:end]
        ids_str = ",".join(str(i) for i in batch_ids)
        data = api_get(f"{API_BASE}/achievements?ids={ids_str}")
        if data and isinstance(data, list):
            for ach in data:
                ach_id = ach.get("id", 0)
                name = ach.get("name", "")
                if ach_id and name:
                    name_index[str(ach_id)] = name
        sys.stdout.write(f"\r  Fetched {end}/{total} ({len(name_index)} names)")
        sys.stdout.flush()

    print(f"\n  Total: {len(name_index)} achievements with names")

    # Step 3: Write output
    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    with open(OUTPUT_PATH, "w") as f:
        json.dump(name_index, f, separators=(",", ":"))
    size_kb = os.path.getsize(OUTPUT_PATH) / 1024
    print(f"  Saved to {OUTPUT_PATH} ({size_kb:.1f} KB)")


if __name__ == "__main__":
    main()
