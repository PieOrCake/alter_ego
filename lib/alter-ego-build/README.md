# alter-ego-build

Encode and decode **Alter Ego AE2 build codes** for Guild Wars 2. Zero dependencies, works in browser and Node.js.

AE2 codes carry a complete build — traits, skills, gear stats, runes, sigils, relics, and weapon types — in ~130–150 characters, fitting within GW2's 199-character chat limit.

## Install

```bash
npm install alter-ego-build
```

Or copy `src/index.js` directly into your project — it's a single file with no dependencies.

## Usage

### Encode

```js
import { encodeAE2 } from 'alter-ego-build';

const code = encodeAE2({
  chatLink: '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]',
  gameMode: 'Raid',      // optional, defaults to 'PvE'
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
  runeId: 24836,     // Superior Rune of Scholar — GW2 item ID
  relicId: 100916,   // Relic of the Thief — exotic item ID
});

console.log(code);
// => "AE2:AwuLDQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA//wQ1BDUENQQ1BDUENQRNAk0CTQJNBDUE..."
```

### Decode

```js
import { decodeAE2 } from 'alter-ego-build';

const build = decodeAE2('AE2:AwuL...');

console.log(build.chatLink);  // '[&DQEeHjElPy5L...]'
console.log(build.gameMode);  // 'Raid'
console.log(build.gear.Helm); // { statId: 1077 }
console.log(build.runeId);    // 24836
console.log(build.relicId);   // 100916
console.log(build.gear.WeaponA1);
// { statId: 1077, sigilId: 24615, weaponType: 'Greatsword', sigil2Id: 24868 }
```

### Minimal encode (traits + skills only, no gear)

```js
const code = encodeAE2({
  chatLink: '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]',
});
```

## API

### `encodeAE2(options) → string`

| Option | Type | Required | Description |
|--------|------|----------|-------------|
| `chatLink` | string | **Yes** | GW2 build template chat link (`[&...]`) |
| `gameMode` | string | No | `PvE` (default), `WvW`, `PvP`, `Raid`, `Fractal`, `Other` |
| `gear` | object | No | Map of slot name → `{ statId, sigilId?, weaponType?, sigil2Id? }` |
| `runeId` | number | No | GW2 item ID of the rune |
| `relicId` | number | No | GW2 **exotic** relic item ID (not legendary) |

### `decodeAE2(code) → object`

Returns `{ version, chatLink, gameMode, gear, runeId, relicId }`.

## Gear Slot Names

| Armor | Weapons | Trinkets |
|-------|---------|----------|
| `Helm` | `WeaponA1` (MH set A) | `Backpack` |
| `Shoulders` | `WeaponA2` (OH set A) | `Accessory1` |
| `Coat` | `WeaponB1` (MH set B) | `Accessory2` |
| `Gloves` | `WeaponB2` (OH set B) | `Amulet` |
| `Leggings` | | `Ring1` |
| `Boots` | | `Ring2` |

## Weapon Types

`Axe`, `Dagger`, `Focus`, `Greatsword`, `Hammer`, `Longbow`, `Mace`, `Pistol`, `Rifle`, `Scepter`, `Shield`, `Shortbow`, `Spear`, `Staff`, `Sword`, `Torch`, `Warhorn`

## Relic IDs

Use **exotic** relic item IDs, not legendary. The legendary relic shares a single item ID across all effects. Example: Relic of the Thief = `100916`, not `101600`.

## Where to find IDs

- **Stat IDs**: [/v2/itemstats](https://api.guildwars2.com/v2/itemstats?ids=all) — e.g. Berserker's = `1077`, Viper's = `1130`
- **Rune/Sigil/Relic IDs**: [/v2/items](https://wiki.guildwars2.com/wiki/API:2/items) — search by name
- **Chat links**: GW2 build templates from the in-game hero panel, or from build editor websites

## License

MIT
