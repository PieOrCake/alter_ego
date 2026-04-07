# Alter Ego

A Guild Wars 2 addon for [Raidcore Nexus](https://raidcore.gg/Nexus) that lets you view and manage all your characters, equipment, builds, and saved build templates — all without logging in to each character.

## AI Notice

This addon has been 100% created in [Windsurf](https://windsurf.com/) using Claude. I understand that some folks have a moral, financial or political objection to creating software using an LLM. I just wanted to make a useful tool for the GW2 community, and this was the only way I could do it.

If an LLM creating software upsets you, then perhaps this repo isn't for you. Move on, and enjoy your day.

## Features

- **Character List** — All characters on your account with profession icons, level, and birthday countdown
  - Sort by name, class, level, age, or birthday — or drag to reorder
  - Compact mode option for denser lists
- **Equipment Panel** — Full paper-doll layout for each character's equipment tabs
  - Equipment icons with rarity-colored borders
  - Skin/transmutation display with original item info
  - Upgrade (sigil/rune) and infusion tooltips
  - Dye color preview swatches
  - Equipment tab switching
- **Build Panel** — Specialization trait grids with animated marching-ant connection lines
  - Specialization portrait and trait icons with tooltips
  - Heal / Utility / Elite skill bar with icons
  - Build tab switching
  - Copy build template chat link to clipboard
- **Build Library** — Import and manage build templates from chat links
  - Full trait grid and skill bar preview
  - Weapon skills display (with Elementalist attunement and Thief dual-wield support)
  - Gear customization: stat combos, runes, sigils, weapon types
  - Filter by profession and game mode, search by name
- **Chat Link Support** — Full import/export of GW2 chat links
  - Item links (with skin, upgrades, infusions)
  - Build template links
  - Skin links
  - Right-click context menu for copying links
- **Hoard & Seek Integration** — Uses [Hoard & Seek](https://github.com/PieOrCake/hoard_and_seek) as a data source for account-wide character and equipment data

## Screenshots

![Equipment panel](screenshots/equipment.png)
![Build library](screenshots/library.png)

## Requirements

- [Raidcore Nexus](https://raidcore.gg/Nexus) (API v6)
- [Hoard & Seek](https://github.com/PieOrCake/hoard_and_seek) addon (provides character data via GW2 API)

## Installation

Copy `AlterEgo.dll` to your GW2 Nexus addons directory:
```
<GW2>/addons/AlterEgo.dll
```

## Building

Cross-compiled for Windows on Linux using MinGW:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Output: `build/AlterEgo.dll`

## Dependencies

- **ImGui 1.80** — Immediate mode GUI (bundled in `lib/imgui/`)
- **nlohmann/json** — JSON parsing (bundled in `lib/nlohmann/`)
- **Nexus API v6** — Raidcore Nexus addon API (header in `include/nexus/`)

## License

MIT
