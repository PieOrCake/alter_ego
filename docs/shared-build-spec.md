# Alter Ego Shared Build Template Specification

**Version:** 2  
**Status:** Stable  
**Last updated:** April 2026

## Overview

Alter Ego Shared Build Templates allow Guild Wars 2 build websites and tools to export complete build configurations — including traits, skills, gear stats, runes, sigils, weapon types, and relics — in a format that can be imported directly into the [Alter Ego](https://github.com/PieOrCake/alter_ego) addon for Guild Wars 2.

Unlike GW2 build chat links (`[&...]`), which only encode traits, skills, pets, and legends, shared build templates carry the **full gear configuration** needed to recreate a build.

## Encoding Formats

Three formats are supported. **AE2 is the preferred format** for in-game chat sharing.

### 1. Raw JSON

Plain JSON object. Suitable for file exports, APIs, and debugging.

```json
{
  "v": 1,
  "name": "Power Dragonhunter",
  "chat_link": "[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]",
  "profession": "Guardian",
  "game_mode": "Raid",
  "notes": "Burst DPS variant with greatsword + scepter/focus",
  "gear": {
    "Helm":       { "stat_id": 1077, "stat": "Berserker's" },
    "Shoulders":  { "stat_id": 1077, "stat": "Berserker's" },
    "Coat":       { "stat_id": 1077, "stat": "Berserker's" },
    "Gloves":     { "stat_id": 1077, "stat": "Berserker's" },
    "Leggings":   { "stat_id": 1077, "stat": "Berserker's" },
    "Boots":      { "stat_id": 1077, "stat": "Berserker's" },
    "WeaponA1":   { "stat_id": 1077, "stat": "Berserker's", "sigil": "Superior Sigil of Force", "weapon": "Greatsword" },
    "WeaponB1":   { "stat_id": 1077, "stat": "Berserker's", "sigil": "Superior Sigil of Accuracy", "weapon": "Scepter" },
    "WeaponB2":   { "stat_id": 1077, "stat": "Berserker's", "sigil": "Superior Sigil of Force", "weapon": "Focus" },
    "Backpack":   { "stat_id": 1077, "stat": "Berserker's" },
    "Accessory1": { "stat_id": 1077, "stat": "Berserker's" },
    "Accessory2": { "stat_id": 1077, "stat": "Berserker's" },
    "Amulet":     { "stat_id": 1077, "stat": "Berserker's" },
    "Ring1":      { "stat_id": 1077, "stat": "Berserker's" },
    "Ring2":      { "stat_id": 1077, "stat": "Berserker's" }
  },
  "rune": "Superior Rune of Scholar",
  "relic": "Relic of the Thief"
}
```

### 2. Compact Code (`AE1:` prefix) — Legacy

Base64-encoded JSON with an `AE1:` prefix. Supported for backward compatibility; **prefer AE2 for new implementations**.

```
AE1:<base64-encoded-json>
```

**Encoding steps:**
1. Serialize the JSON object to a string (minified, no extra whitespace)
2. Base64-encode the UTF-8 bytes using standard Base64 (RFC 4648, alphabet `A-Z a-z 0-9 + /`, `=` padding)
3. Prepend `AE1:`

**Decoding steps:**
1. Strip the `AE1:` prefix
2. Base64-decode to get UTF-8 bytes
3. Parse as JSON

### 3. Compact Binary (`AE2:` prefix) — Preferred

Binary format with an `AE2:` prefix, designed to fit complete builds (traits, skills, gear stats, runes, sigils, relics) within GW2's 199-character chat limit. Typical output is 130–150 base64 characters.

```
AE2:<base64-encoded-binary>
```

#### Binary Layout (little-endian)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 1 | `version` | Always `2`. |
| 1 | 1 | `flags` | Bit field (see below). |
| 2 | 1 | `build_link_len` | Length of the raw build link bytes. |
| 3 | N | `build_link` | Raw bytes of the GW2 build chat link (the base64-decoded `[&...]` payload, excluding the `[&` prefix and `]` suffix). |

After `build_link`, optional sections follow based on `flags`:

#### Flags Byte

| Bits | Mask | Field |
|------|------|-------|
| 0–2 | `0x07` | Game mode: 0=PvE, 1=WvW, 2=PvP, 3=Raid, 4=Fractal |
| 3 | `0x08` | Has gear data |
| 4 | `0x10` | Has rune ID |
| 5 | `0x20` | Has relic ID |

#### Gear Data (if bit 3 set)

| Size | Field | Description |
|------|-------|-------------|
| 2 | `gear_mask` | 16-bit bitmask indicating which gear slots are present. |
| 2 × popcount(gear_mask) | `stat_ids` | One `uint16` itemstat ID per set bit, in order from bit 0 to bit 15. |

**Gear slot indices:**

| Index | Slot | Index | Slot |
|-------|------|-------|------|
| 0 | Helm | 8 | WeaponB1 |
| 1 | Shoulders | 9 | WeaponB2 |
| 2 | Coat | 10 | Back |
| 3 | Gloves | 11 | Accessory1 |
| 4 | Leggings | 12 | Accessory2 |
| 5 | Boots | 13 | Amulet |
| 6 | WeaponA1 | 14 | Ring1 |
| 7 | WeaponA2 | 15 | Ring2 |

#### Rune ID (if bit 4 set)

| Size | Field | Description |
|------|-------|-------------|
| 4 | `rune_id` | `uint32` GW2 item ID of the rune. Resolvable against `/v2/items`. |

#### Relic ID (if bit 5 set)

| Size | Field | Description |
|------|-------|-------------|
| 4 | `relic_id` | `uint32` GW2 item ID of the **exotic** relic. See [Relic IDs](#relic-ids). |

#### Sigil IDs (after rune/relic, for each weapon slot in gear_mask)

For each weapon slot present in `gear_mask` (bits 6–9, in order):

| Size | Field | Description |
|------|-------|-------------|
| 4 | `sigil_id` | `uint32` GW2 item ID of the sigil. |

#### Encoding Steps

1. Build the binary payload as described above
2. Base64-encode the bytes (RFC 4648, `A-Z a-z 0-9 + /`, `=` padding)
3. Prepend `AE2:`

#### Decoding Steps

1. Strip the `AE2:` prefix
2. Base64-decode to get raw bytes
3. Parse the binary layout as described above
4. Decode the `build_link` bytes as a GW2 build template to extract traits, skills, pets, legends
5. Resolve item IDs against `/v2/items` for display names

## JSON Schema Reference

### Top-level fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `v` | integer | **Yes** | Schema version. Currently `1`. |
| `name` | string | **Yes** | Display name for the build. |
| `chat_link` | string | **Yes** | GW2 build template chat link (`[&...]`). |
| `profession` | string | Recommended | Profession name. See [Profession Names](#profession-names). |
| `game_mode` | string | No | Game mode tag. See [Game Modes](#game-modes). Defaults to `"PvE"`. |
| `notes` | string | No | Free-text notes about the build. |
| `gear` | object | No | Gear configuration. See [Gear Object](#gear-object). |
| `rune` | string | No | Shared rune name across all armor pieces. |
| `relic` | string | No | Relic name. |

### Gear Object

A map of **slot name** → **gear slot object**. Only include slots that have data.

#### Slot Names

**Armor:**
`Helm`, `Shoulders`, `Coat`, `Gloves`, `Leggings`, `Boots`

**Weapons:**
`WeaponA1` (main hand set A), `WeaponA2` (off hand set A), `WeaponB1` (main hand set B), `WeaponB2` (off hand set B)

**Trinkets:**
`Backpack`, `Accessory1`, `Accessory2`, `Amulet`, `Ring1`, `Ring2`

**Other:**
`Relic` (not typically included in gear — use the top-level `relic` field instead)

#### Gear Slot Object

| Field | Type | Description |
|-------|------|-------------|
| `stat_id` | integer | GW2 API itemstat ID from [`/v2/itemstats`](https://wiki.guildwars2.com/wiki/API:2/itemstats). |
| `stat` | string | Human-readable stat combo name (e.g., `"Berserker's"`, `"Viper's"`). |
| `rune` | string | Rune name (armor slots only). |
| `sigil` | string | Sigil name (weapon slots only). |
| `infusion` | string | Infusion name. |
| `weapon` | string | Weapon type (weapon slots only). See [Weapon Types](#weapon-types). |

All fields are optional. Include whichever data is available.

If `stat_id` is provided, Alter Ego will use it for exact stat matching. If only `stat` (the name) is provided, it serves as a display label.

## Relic IDs

Relic IDs in AE2 must be **exotic relic item IDs**, not legendary. The legendary relic is a single item that can emulate any exotic relic, but its item ID is shared across all effects. Exotic relics each have a unique item ID that identifies the specific relic effect.

For example: `Relic of the Thief` = item ID `100916`, not the legendary relic ID `101600`.

Relic item IDs can be resolved against [`/v2/items`](https://wiki.guildwars2.com/wiki/API:2/items). A full list of relics is available at the [GW2 Wiki](https://wiki.guildwars2.com/wiki/Relic).

## Reference Values

### Profession Names

Must match the GW2 API exactly:

`Guardian`, `Warrior`, `Engineer`, `Ranger`, `Thief`, `Elementalist`, `Mesmer`, `Necromancer`, `Revenant`

### Game Modes

`PvE`, `WvW`, `PvP`, `Raid`, `Fractal`, `Other`

### Weapon Types

Must match the GW2 API `/v2/items` weapon detail `type` field:

`Axe`, `Dagger`, `Focus`, `Greatsword`, `Hammer`, `Longbow`, `Mace`, `Pistol`, `Rifle`, `Scepter`, `Shield`, `Shortbow`, `Spear`, `Staff`, `Sword`, `Torch`, `Warhorn`

### Common Stat IDs

| Stat Combo | ID |
|------------|-----|
| Berserker's | 1077 |
| Viper's | 1130 |
| Harrier's | 1134 |
| Diviner's | 1271 |
| Ritualist's | 1393 |
| Dragon's | 1379 |
| Celestial | 1118 |
| Marauder | 1128 |

Full list available from the [GW2 API](https://api.guildwars2.com/v2/itemstats?ids=all).

## Implementation Guide

### For build websites (export)

To add an "Export to Alter Ego" button:

1. Construct the JSON object with the build's chat link, gear, and metadata
2. Encode as either:
   - **Compact code** — for a "Copy to clipboard" button (paste into Alter Ego's import field)
   - **JSON** — for a downloadable file or API response
3. The user pastes the code into Alter Ego's Build Library → Import Build field

**Minimal example** (traits + skills only, no gear):

```json
{
  "v": 1,
  "name": "My Build",
  "chat_link": "[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]"
}
```

**Full example** (with gear):

```json
{
  "v": 1,
  "name": "Power Dragonhunter",
  "chat_link": "[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]",
  "profession": "Guardian",
  "game_mode": "Raid",
  "gear": {
    "Helm":     { "stat_id": 1077, "stat": "Berserker's" },
    "Coat":     { "stat_id": 1077, "stat": "Berserker's" },
    "WeaponA1": { "stat_id": 1077, "stat": "Berserker's", "weapon": "Greatsword", "sigil": "Superior Sigil of Force" }
  },
  "rune": "Superior Rune of Scholar",
  "relic": "Relic of the Thief"
}
```

You do not need to include every slot — only include slots you have data for.

### For addon/tool developers (import)

1. Check if the input starts with `AE2:` → decode base64, parse binary (see [Compact Binary](#3-compact-binary-ae2-prefix--preferred))
2. Check if the input starts with `AE1:` → decode base64, parse JSON
3. Check if the input starts with `{` → parse as raw JSON
4. Otherwise → treat as a GW2 chat link (existing behavior)
5. Validate version field for compatibility (`v` for JSON, byte 0 for AE2)
6. Decode the build link to get traits, skills, pets, legends
7. Apply gear/rune/sigil/relic data

## Versioning

The `v` field indicates the schema version. Importers should:
- Accept `v: 1` as defined in this document
- Ignore unknown fields gracefully (forward compatibility)
- Reject documents with `v` values they don't understand

Future versions will be backwards-compatible where possible. Breaking changes will increment the version number.

## Contact

- **Repository:** [github.com/PieOrCake/alter_ego](https://github.com/PieOrCake/alter_ego)
- **Homepage:** [pie.rocks.cc/projects/alter-ego](https://pie.rocks.cc/projects/alter-ego/)
