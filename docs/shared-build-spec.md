# Alter Ego Shared Build Template Specification

**Version:** 3  
**Status:** Stable  
**Last updated:** April 2026

## Overview

Alter Ego Shared Build Templates allow Guild Wars 2 build websites and tools to export complete build configurations — including traits, skills, gear stats, runes, sigils, weapon types, and relics — in a format that can be imported directly into the [Alter Ego](https://github.com/PieOrCake/alter_ego) addon for Guild Wars 2.

Unlike GW2 build chat links (`[&...]`), which only encode traits, skills, pets, and legends, shared build templates carry the **full gear configuration** needed to recreate a build.

## Quick Start (JavaScript)

The easiest way to add AE2 support to a website is with the [`alter-ego-build`](https://www.npmjs.com/package/alter-ego-build) library — zero dependencies, works in browser and Node.js. Use the [**AE2 Build Viewer**](https://pie.rocks.cc/tools/ae2-playground/) to verify your output visually.

```bash
npm install alter-ego-build
```

### Encode a build

```js
import { encodeAE2 } from 'alter-ego-build';

const code = encodeAE2({
  chatLink: '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]',
  gameMode: 'Raid',
  gear: {
    Helm:       { statId: 1077 },
    Shoulders:  { statId: 1077 },
    Coat:       { statId: 1077 },
    Gloves:     { statId: 1077 },
    Leggings:   { statId: 1077 },
    Boots:      { statId: 1077 },
    WeaponA1:   { statId: 1077, sigilId: 24615, weaponType: 'Greatsword', sigil2Id: 24868 },
    WeaponB1:   { statId: 1077, sigilId: 24618, weaponType: 'Scepter' },
    WeaponB2:   { statId: 1077, sigilId: 24615, weaponType: 'Focus' },
    Backpack:   { statId: 1077 },
    Accessory1: { statId: 1077 },
    Accessory2: { statId: 1077 },
    Amulet:     { statId: 1077 },
    Ring1:      { statId: 1077 },
    Ring2:      { statId: 1077 },
  },
  runeId: 24836,     // Superior Rune of Scholar
  relicId: 100916,   // Relic of the Thief (exotic ID)
});
// => "AE2:AwuL..." — ready for clipboard or GW2 chat
```

### Decode a build

```js
import { decodeAE2 } from 'alter-ego-build';

const build = decodeAE2(code);
// { chatLink, gameMode, gear, runeId, relicId, version }
```

See the [library README](https://www.npmjs.com/package/alter-ego-build) for the full API reference.

If you need to implement AE2 in another language, the binary format is documented below.

## Format

Compact binary format with an `AE2:` prefix, designed to fit complete builds (traits, skills, gear stats, runes, sigils, relics) within GW2's 199-character chat limit. Typical output is 130–150 base64 characters.

```
AE2:<base64-encoded-binary>
```

#### Binary Layout (little-endian)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 1 | `version` | Format version. Current: `3` (also accepts `2` for backward compat). |
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

#### Weapon Slot Data (after rune/relic, for each weapon slot in gear_mask)

For each weapon slot present in `gear_mask` (bits 6–9, in order):

| Size | Field | Description |
|------|-------|-------------|
| 4 | `sigil_id` | `uint32` GW2 item ID of the sigil. 0 if none. |
| 2 | `weapon_type_id` | `uint16` weapon type identifier. See [Weapon Type IDs](#weapon-type-ids). 0 if none. |
| 4 | `sigil2_id` | `uint32` GW2 item ID of the second sigil (for two-handed weapons). 0 if none. *(v3 only — not present in v2)* |

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

#### Weapon Type IDs

These match the GW2 API item `type` values:

| ID | Weapon | ID | Weapon |
|----|--------|----|--------|
| 5 | Axe | 86 | Scepter |
| 35 | Longbow | 87 | Shield |
| 47 | Dagger | 89 | Staff |
| 49 | Focus | 90 | Sword |
| 50 | Greatsword | 102 | Torch |
| 51 | Hammer | 103 | Warhorn |
| 53 | Mace | 107 | Shortbow |
| 54 | Pistol | 265 | Spear |
| 85 | Rifle | 0 | (none) |

#### Version Differences

| Version | Per-weapon-slot size | Fields |
|---------|---------------------|--------|
| 2 | 6 bytes | `sigil_id` (4) + `weapon_type_id` (2) |
| 3 | 10 bytes | `sigil_id` (4) + `weapon_type_id` (2) + `sigil2_id` (4) |

Decoders should check byte 0 (`version`) to determine the per-weapon-slot layout. Version 2 codes are still accepted for backward compatibility.

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

### For build websites (JavaScript)

The recommended approach — no binary packing required:

1. `npm install alter-ego-build`
2. Call `encodeAE2()` with the build's chat link, gear stat IDs, rune/relic/sigil item IDs, and weapon types
3. Verify your output with the [AE2 Build Viewer](https://pie.rocks.cc/tools/ae2-playground/) — paste the code to see the full build rendered visually
4. Present the resulting string as a "Copy to clipboard" button
5. The user pastes the code into Alter Ego's Build Library → Import Build field, or shares it in GW2 chat

### For other languages (manual encoding)

To implement AE2 without the JS library:

1. Build the binary payload as described in [Binary Layout](#binary-layout-little-endian)
2. Base64-encode the bytes and prepend `AE2:`

### For addon/tool developers (import)

1. Check if the input starts with `AE2:` → decode base64, parse binary
2. Check byte 0 (`version`) to determine per-weapon-slot layout (v2 or v3)
3. Decode the `build_link` bytes as a GW2 build template to extract traits, skills, pets, legends
4. Resolve item IDs against `/v2/items` for display names
5. Apply gear/rune/sigil/relic data

## Contact

- **Repository:** [github.com/PieOrCake/alter_ego](https://github.com/PieOrCake/alter_ego)
- **Homepage:** [pie.rocks.cc/projects/alter-ego](https://pie.rocks.cc/projects/alter-ego/)
