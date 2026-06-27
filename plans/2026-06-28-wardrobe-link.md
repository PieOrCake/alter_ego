# Wardrobe ("Fashion") link from equipment page

## Goal
On each character equipment tab, generate a native GW2 **wardrobe template chat link**
(`[&...]`, link type `0x0F`) from the armor + weapons shown — skin per slot (transmuted
skin if present, else the item's default skin) plus armor dyes. Exposed via a **Fashion**
button → popup menu with **Copy Code** (live) and **Preview** (stubbed, pending a Pie UI
event integration prompt to be supplied later).

## Scope
- Armor (Helm, Shoulders, Coat→chest, Gloves, Leggings, Boots): skin + up to 4 dyes.
- Backpack: skin + dyes.
- Terrestrial weapons: WeaponA1/A2 → weapon_a_main/off, WeaponB1/B2 → weapon_b_main/off.
- Excluded (per "not underwater"): aquabreather, aquatic weapons, outfit. Visibility flags = 0 (all shown).

## Existing infrastructure
- `LINK_WARDROBE_TMPL = 0x0F` and `DecodedWardrobeLink` already defined in `src/ChatLink.h`.
- `ChatLink::DecodeWardrobe` already implemented (`src/ChatLink.cpp`) — the exact byte order
  to mirror. **No encoder exists yet** and the decoder is currently unused (dormant).

## Changes
1. **Data**: add `uint32_t default_skin` to `ItemInfo` (`GW2API.h`); parse `default_skin`
   from `/v2/items` (`GW2API.cpp` ~1837); persist in `SaveItemCache`/load (`GW2API.cpp`
   ~3093 / ~3273). Free — same response we already cache. Old caches resolve lazily on refetch.
2. **Encoder**: `ChatLink::EncodeWardrobe(const DecodedWardrobeLink&)` — byte-inverse of
   `DecodeWardrobe`, so the two round-trip. Declare in `ChatLink.h`.
3. **Glue**: `BuildWardrobeLink(const Character&, int tab)` static helper in `dllmain.cpp` —
   fills a `DecodedWardrobeLink` from the tab's slots (skin = `eq.skin ? eq.skin :
   ItemInfo.default_skin`; up to 4 dyes), returns `EncodeWardrobe(...)`.
4. **UI**: "Fashion" chip on the equipment panel (near the tab selector) → `BeginPopup`
   menu mirroring the Build Library Share popup: **Copy Code** (`CopyToClipboard` + alert),
   **Preview** (disabled `Selectable`, wired to Pie UI later).

## Verify
- Round-trip a real in-game wardrobe link through `DecodeWardrobe` to confirm field order.
- User pastes a generated link in-game to confirm it previews correctly.

## Notes / follow-ups
- Skins are `uint16` in this format (matches the real link); current GW2 skin IDs fit.
- Missing default-skin (item not yet cached) → that slot encodes 0 until cache fills (lazy).
- Backpack included as part of the visible look — drop if undesired.
