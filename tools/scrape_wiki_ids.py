#!/usr/bin/env python3
"""Scrape GW2 Wiki for rune, sigil, and relic item IDs."""
import urllib.request
import urllib.parse
import json
import re
import time

RUNES = [
    "Superior Rune of the Scholar",
    "Superior Rune of the Eagle",
    "Superior Rune of Strength",
    "Superior Rune of the Pack",
    "Superior Rune of the Monk",
    "Superior Rune of the Warrior",
    "Superior Rune of Divinity",
    "Superior Rune of the Firebrand",
    "Superior Rune of the Berserker",
    "Superior Rune of the Dragonhunter",
    "Superior Rune of the Reaper",
    "Superior Rune of the Renegade",
    "Superior Rune of the Herald",
    "Superior Rune of the Scourge",
    "Superior Rune of the Mirage",
    "Superior Rune of the Weaver",
    "Superior Rune of the Deadeye",
    "Superior Rune of the Soulbeast",
    "Superior Rune of the Nightmare",
    "Superior Rune of Tormenting",
    "Superior Rune of the Trapper",
    "Superior Rune of the Undead",
    "Superior Rune of Balthazar",
    "Superior Rune of the Baelfire",
    "Superior Rune of Surging",
    "Superior Rune of the Water",
    "Superior Rune of the Tempest",
    "Superior Rune of Sanctuary",
    "Superior Rune of the Dolyak",
    "Superior Rune of Durability",
    "Superior Rune of the Defender",
    "Superior Rune of Vampirism",
    "Superior Rune of Speed",
    "Superior Rune of the Traveler",
    "Superior Rune of Aristocracy",
    "Superior Rune of Rage",
    "Superior Rune of Altruism",
    "Superior Rune of the Sunless",
    "Superior Rune of Radiance",
    "Superior Rune of the Citadel",
]

SIGILS = [
    "Superior Sigil of Force",
    "Superior Sigil of Impact",
    "Superior Sigil of Accuracy",
    "Superior Sigil of the Night",
    "Superior Sigil of Undead Slaying",
    "Superior Sigil of Demon Slaying",
    "Superior Sigil of Air",
    "Superior Sigil of Fire",
    "Superior Sigil of Earth",
    "Superior Sigil of Geomancy",
    "Superior Sigil of Doom",
    "Superior Sigil of Torment",
    "Superior Sigil of Malice",
    "Superior Sigil of Smoldering",
    "Superior Sigil of Venom",
    "Superior Sigil of Agony",
    "Superior Sigil of Bursting",
    "Superior Sigil of Concentration",
    "Superior Sigil of Transference",
    "Superior Sigil of Renewal",
    "Superior Sigil of Blood",
    "Superior Sigil of Strength",
    "Superior Sigil of Severance",
    "Superior Sigil of Energy",
    "Superior Sigil of Paralyzation",
    "Superior Sigil of Absorption",
    "Superior Sigil of Cleansing",
    "Superior Sigil of Draining",
    "Superior Sigil of Exploitation",
    "Superior Sigil of Demons",
    "Superior Sigil of Serpent Slaying",
    "Superior Sigil of Courage",
]

RELICS = [
    "Relic of the Eagle",
    "Relic of Fireworks",
    "Relic of the Thief",
    "Relic of Cerus",
    "Relic of Isgarren",
    "Relic of the Brawler",
    "Relic of the Claw",
    "Relic of the Deadeye",
    "Relic of the Dragonhunter",
    "Relic of Mount Balrior",
    "Relic of Peitha",
    "Relic of Lyhr",
    "Relic of Fire",
    "Relic of Dagda",
    "Relic of the Fractal",
    "Relic of Akeem",
    "Relic of the Aristocracy",
    "Relic of the Afflicted",
    "Relic of the Nightmare",
    "Relic of the Krait",
    "Relic of the Sorcerer",
    "Relic of the Mirage",
    "Relic of Thorns",
    "Relic of the Blightbringer",
    "Relic of Nourys",
    "Relic of the Scourge",
    "Relic of the Demon Queen",
    "Relic of Mosyn",
    "Relic of the Biomancer",
    "Relic of the First Revenant",
    "Relic of Durability",
    "Relic of the Monk",
    "Relic of the Flock",
    "Relic of Castora",
    "Relic of the Water",
    "Relic of the Defender",
    "Relic of Vampirism",
    "Relic of Dwayna",
    "Relic of the Centaur",
    "Relic of Mercy",
    "Relic of Nayos",
    "Relic of Karakosa",
    "Relic of the Nautical Beast",
    "Relic of the Living City",
    "Relic of the Wayfinder",
    "Relic of Speed",
    "Relic of the Adventurer",
    "Relic of Evasion",
    "Relic of the Cavalier",
    "Relic of Resistance",
    "Relic of the Reaper",
    "Relic of the Ice",
    "Relic of Antitoxin",
    "Relic of the Sunless",
    "Relic of the Citadel",
    "Relic of the Trooper",
    "Relic of the Herald",
    "Relic of the Firebrand",
    "Relic of the Weaver",
]


def fetch_item_id(name):
    """Fetch item ID from GW2 Wiki by scraping the page for the API link."""
    wiki_name = name.replace(" ", "_")
    url = f"https://wiki.guildwars2.com/wiki/{urllib.parse.quote(wiki_name)}"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "AlterEgo-Scraper/1.0"})
        with urllib.request.urlopen(req, timeout=10) as resp:
            html = resp.read().decode("utf-8")
        # Look for the API item ID - it's in a link like api.guildwars2.com/v2/items?ids=XXXXX
        m = re.search(r'api\.guildwars2\.com/v2/items\?ids=(\d+)', html)
        if m:
            return int(m.group(1))
        # Also try the pattern: Has game id = XXXXX
        m = re.search(r'game[_ ]id["\s:=]+(\d+)', html, re.IGNORECASE)
        if m:
            return int(m.group(1))
    except Exception as e:
        print(f"  ERROR fetching {name}: {e}")
    return None


def scrape_category(items, label):
    results = {}
    print(f"\n=== {label} ({len(items)} items) ===")
    for name in items:
        item_id = fetch_item_id(name)
        if item_id:
            results[name] = item_id
            print(f"  {name}: {item_id}")
        else:
            print(f"  {name}: NOT FOUND")
        time.sleep(0.3)  # Be nice to the wiki
    return results


if __name__ == "__main__":
    rune_ids = scrape_category(RUNES, "RUNES")
    sigil_ids = scrape_category(SIGILS, "SIGILS")
    relic_ids = scrape_category(RELICS, "RELICS")

    print("\n\n=== C++ RUNE TABLE ===")
    for name, item_id in rune_ids.items():
        # Find the bonus from the existing list
        print(f'    {{"{name}", {item_id}, "..."}},')

    print("\n=== C++ SIGIL TABLE ===")
    for name, item_id in sigil_ids.items():
        print(f'    {{"{name}", {item_id}, "..."}},')

    print("\n=== C++ RELIC TABLE ===")
    for name, item_id in relic_ids.items():
        print(f'    {{"{name}", {item_id}, "..."}},')

    # Save as JSON too
    all_data = {"runes": rune_ids, "sigils": sigil_ids, "relics": relic_ids}
    with open("wiki_item_ids.json", "w") as f:
        json.dump(all_data, f, indent=2)
    print(f"\nSaved to wiki_item_ids.json")
