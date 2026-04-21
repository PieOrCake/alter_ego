#!/usr/bin/env python3
"""Fetch rune/sigil/relic item IDs from GW2 API by scanning ID ranges."""
import urllib.request
import json
import sys

# Names we need to find IDs for
RUNE_NAMES = {
    "Superior Rune of the Scholar", "Superior Rune of the Eagle", "Superior Rune of Strength",
    "Superior Rune of the Pack", "Superior Rune of the Monk", "Superior Rune of the Warrior",
    "Superior Rune of Divinity", "Superior Rune of the Firebrand", "Superior Rune of the Berserker",
    "Superior Rune of the Dragonhunter", "Superior Rune of the Reaper", "Superior Rune of the Renegade",
    "Superior Rune of the Herald", "Superior Rune of the Scourge", "Superior Rune of the Mirage",
    "Superior Rune of the Weaver", "Superior Rune of the Deadeye", "Superior Rune of the Soulbeast",
    "Superior Rune of the Nightmare", "Superior Rune of Tormenting", "Superior Rune of the Trapper",
    "Superior Rune of the Undead", "Superior Rune of Balthazar", "Superior Rune of the Baelfire",
    "Superior Rune of Surging", "Superior Rune of the Water", "Superior Rune of the Tempest",
    "Superior Rune of Sanctuary", "Superior Rune of the Dolyak", "Superior Rune of Durability",
    "Superior Rune of the Defender", "Superior Rune of Vampirism", "Superior Rune of Speed",
    "Superior Rune of the Traveler", "Superior Rune of Aristocracy", "Superior Rune of Rage",
    "Superior Rune of Altruism", "Superior Rune of the Sunless", "Superior Rune of Radiance",
    "Superior Rune of the Citadel",
}

SIGIL_NAMES = {
    "Superior Sigil of Force", "Superior Sigil of Impact", "Superior Sigil of Accuracy",
    "Superior Sigil of the Night", "Superior Sigil of Undead Slaying", "Superior Sigil of Demon Slaying",
    "Superior Sigil of Air", "Superior Sigil of Fire", "Superior Sigil of Earth",
    "Superior Sigil of Geomancy", "Superior Sigil of Doom", "Superior Sigil of Torment",
    "Superior Sigil of Malice", "Superior Sigil of Smoldering", "Superior Sigil of Venom",
    "Superior Sigil of Agony", "Superior Sigil of Bursting", "Superior Sigil of Concentration",
    "Superior Sigil of Transference", "Superior Sigil of Renewal", "Superior Sigil of Blood",
    "Superior Sigil of Strength", "Superior Sigil of Severance", "Superior Sigil of Energy",
    "Superior Sigil of Paralyzation", "Superior Sigil of Absorption", "Superior Sigil of Cleansing",
    "Superior Sigil of Draining", "Superior Sigil of Exploitation", "Superior Sigil of Demons",
    "Superior Sigil of Serpent Slaying", "Superior Sigil of Courage",
}

# We want ALL exotic relics (not a fixed list)
ALL_NAMES = RUNE_NAMES | SIGIL_NAMES

def fetch_batch(ids):
    """Fetch item details from GW2 API for a batch of IDs."""
    ids_str = ",".join(str(i) for i in ids)
    url = f"https://api.guildwars2.com/v2/items?ids={ids_str}"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "AlterEgo/1.0"})
        with urllib.request.urlopen(req, timeout=15) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except Exception as e:
        return []

def scan_range(start, end, batch_size=200):
    """Scan a range of IDs and return name->id mapping for matches."""
    found = {}
    relics = {}
    for i in range(start, end, batch_size):
        batch = list(range(i, min(i + batch_size, end)))
        items = fetch_batch(batch)
        for item in items:
            name = item.get("name", "")
            item_id = item.get("id", 0)
            item_type = item.get("type", "")
            rarity = item.get("rarity", "")
            if name in ALL_NAMES:
                found[name] = item_id
            # Also collect all exotic relics
            if item_type == "Relic" and rarity == "Exotic":
                relics[name] = item_id
        sys.stdout.write(f"\r  Scanning {i}-{min(i+batch_size, end)}... found {len(found)} runes/sigils, {len(relics)} relics")
        sys.stdout.flush()
    print()
    return found, relics

if __name__ == "__main__":
    all_found = {}
    all_relics = {}

    # Base game runes/sigils (24000-25000 range)
    print("Scanning base game range (24000-25000)...")
    f, r = scan_range(24000, 25000)
    all_found.update(f); all_relics.update(r)

    # HoT/PoF expansion runes (67300-67400, 83300-83400)
    print("Scanning HoT range (67300-67400)...")
    f, r = scan_range(67300, 67400)
    all_found.update(f); all_relics.update(r)

    print("Scanning PoF range (83300-83400)...")
    f, r = scan_range(83300, 83400)
    all_found.update(f); all_relics.update(r)

    # EoD/SotO runes (91000-92000, 95000-96000, 97000-98000)
    for start in [91000, 95000, 97000, 99000]:
        print(f"Scanning range ({start}-{start+1000})...")
        f, r = scan_range(start, start + 1000)
        all_found.update(f); all_relics.update(r)

    # Relics (99000-107000)
    for start in [100000, 101000, 102000, 103000, 104000, 105000, 106000, 107000]:
        print(f"Scanning relic range ({start}-{start+1000})...")
        f, r = scan_range(start, start + 1000)
        all_found.update(f); all_relics.update(r)

    # Report
    print(f"\n=== Found {len(all_found)} runes/sigils ===")
    runes = {n: i for n, i in all_found.items() if "Rune" in n}
    sigils = {n: i for n, i in all_found.items() if "Sigil" in n}

    missing = ALL_NAMES - set(all_found.keys())
    if missing:
        print(f"\n!!! MISSING {len(missing)} items: {missing}")

    # Output JSON
    data = {"runes": runes, "sigils": sigils, "relics": all_relics}
    with open("tools/wiki_item_ids.json", "w") as f:
        json.dump(data, f, indent=2, sort_keys=True)
    print(f"\nSaved to tools/wiki_item_ids.json")
    print(f"  Runes: {len(runes)}, Sigils: {len(sigils)}, Relics: {len(all_relics)}")
