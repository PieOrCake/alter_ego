# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Collaboration

When the user says **"discuss"** or **"suggest"**, respond in words only — no code, no edits. Talk through the idea and wait for explicit confirmation before implementing anything.

After every code change, always build the DLL (`cd build && make -j$(nproc)`) and confirm it compiled cleanly.

## Build

Cross-compiled Windows DLL from Linux using MinGW:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Output: `build/AlterEgo.dll` — copy to `<GW2>/addons/AlterEgo.dll` to install.

Requires `x86_64-w64-mingw32-g++` and `x86_64-w64-mingw32-windres`. Build type defaults to `Release` (`-Os`).

## Architecture

**AlterEgo** is a [Raidcore Nexus](https://raidcore.gg/Nexus) addon for Guild Wars 2 — a Windows DLL loaded by the Nexus addon framework. It has no test suite; validation is manual in-game.

### Entry point and UI

`src/dllmain.cpp` is the entire UI layer. It contains:
- Nexus lifecycle hooks (`AddonLoad`, `AddonUnload`, `AddonRender`, etc.)
- All ImGui tab rendering (Characters, Equipment, Builds, Build Library, Skinventory, Vault & Clears, Achievements)
- The achievement system (groups, categories, progress, pinned tracker, alerts, waypoints, hero challenges)
- Build library UI (import, export, AE2 relay, inline editing, drag-to-reorder)
- MumbleLink identity parsing for current character detection
- Integration with the "Events: Chat" addon to detect build links in game chat

`src/AddonShared.h` declares the global `APIDefs` (`AddonAPI_t*`) handle and a few shared utilities (`CopyToClipboard`, `RenderSectionHeader`, `GetEffectiveAccountName`) whose implementations live in `dllmain.cpp`.

### Data layer

`src/GW2API.h` / `src/GW2API.cpp` — static class `AlterEgo::GW2API`. Owns all application state:
- Character data, equipment, build tabs (sourced from Hoard & Seek via events)
- All GW2 public API caches: items, skins, specs, traits, skills, dye colors, itemstats, professions, weapon palettes
- Build library (saved builds as `SavedBuild` structs, persisted to JSON)
- Multi-account support (account list, current account resolution from MumbleLink char)
- Skin unlock status and item location queries (via H&S cache)

All data structures (`Character`, `EquipmentItem`, `BuildTemplate`, `SavedBuild`, etc.) are defined in `GW2API.h`.

### Hoard & Seek integration

Character data (equipment, builds, hero points) is **not fetched directly** from the GW2 API. It comes from the [Hoard & Seek](https://github.com/PieOrCake/hoard_and_seek) companion addon via Nexus inter-addon events. `include/HoardAndSeekAPI.h` is the event interface header.

**Critical threading rule:** Nexus `Events_Raise()` dispatches synchronously on the calling thread — the response handler fires *before* `Events_Raise` returns. Never hold a mutex when calling `Events_Raise` for a query event, or you will deadlock. See `include/HoardAndSeekAPI.h` for the full model and retry pattern for `HOARD_STATUS_PENDING`.

Public GW2 API endpoints (items, skins, specs, traits, skills, etc.) are fetched directly via WinINet in `src/HttpClient.cpp`.

### Other modules

| File | Purpose |
|------|---------|
| `src/Clears.cpp` | Wizard's Vault objectives + raid/fractal clears tracking |
| `src/ChatLink.cpp` | GW2 build chat link (`[&...]`) encode/decode |
| `src/OwnedSkins.cpp` | Skin ownership state management |
| `src/SkinCache.cpp` | Persistent skin data cache |
| `src/Commerce.cpp` | Trading post price lookups |
| `src/WikiImage.cpp` | Wiki image fetching for skins |
| `src/IconManager.cpp` | Nexus texture loading/management |
| `src/SpecDescriptions.h` | Static specialization description strings |

### AE2 build code format

`docs/shared-build-spec.md` documents the `AE2:` compact binary format for sharing full builds (traits + gear + sigils + rune + relic). Version 3 is current; version 2 is accepted for backward compatibility. The JS reference implementation is the [`alter-ego-build`](https://www.npmjs.com/package/alter-ego-build) npm package.

### Data files and tools

`data/` — static JSON files loaded at runtime:
- `achievement_names.json` — achievement ID→name map
- `achievement_waypoints.json` — waypoints per achievement
- `hero_challenges.json` — hero challenge locations

`tools/` — Python scripts to regenerate those data files from the GW2 API and wiki. Run them manually when GW2 content updates require it.

### Dependencies (all bundled)

- `lib/imgui/` — ImGui 1.80
- `lib/nlohmann/` (or `lib/nlohmann_json.hpp`) — nlohmann/json
- `include/nexus/` — Nexus addon API v6 headers
- `include/HoardAndSeekAPI.h` — H&S inter-addon event interface
