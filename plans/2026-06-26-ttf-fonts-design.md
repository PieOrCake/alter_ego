# TrueType Font Support — Design

Date: 2026-06-26
Branch: `feature/ttf-fonts` (off `main`)
Status: design, pre-implementation
Source brief: [ttf-support-prompt.md](../ttf-support-prompt.md)

## Goal

Let the user choose a real TrueType font, baked crisp at an arbitrary pixel size, applied
globally to all AlterEgo text — replacing the blurry ambient ImGui default. One global font
choice; no per-window overrides.

## Project realities (why this is simpler than the generic brief)

- AlterEgo does **not** use Nexus's baked `FontUI`/`FontBig` or any S/M/L/XL preset system.
  It renders everything in the ambient ImGui font (`NexusLinkData.Font`) with only a couple
  of one-off `SetWindowFontScale` calls. → The brief's **legacy-preset migration (§6) does
  not apply**, and the OOTB invariant is trivial: default = push nothing.
- It already has a **Nexus Options page** (`AddonOptions()`, registered via
  `GUI_Register(RT_OptionsRender, AddonOptions)`) — the home for the font UI. No new tab.
- The Nexus font API is present as the brief describes: `Fonts_AddFromFile`, `Fonts_Get`,
  `Fonts_Release`, `Fonts_AddFromResource` are fields of `AddonAPI_t`; `Fonts_AddFromMemory`
  is a typedef only (not a field) — so we must load from a **file path**.
- `HttpClient` already has `DownloadToFile` (WinINet) — reuse it for the bundled font fetch.
- Nexus overrides every addon font's glyph ranges (ASCII + Latin-Extended + Nexus's own
  strings). **TTF does NOT unlock symbols/emoji** — already proven in this codebase (the
  `●` in the build editor's EDITING badge rendered as `?`). No symbol expectations.

## Decisions (settled in brainstorming)

1. **Global font only** — one font for all AlterEgo text; no per-window overrides.
2. **Bundled face = Inter Regular, downloaded on first use** (reusing `DownloadToFile`),
   cached to disk across launches.
3. **PushFont, not SetWindowFontScale.** A TTF baked at the chosen px is applied with
   `ImGui::PushFont` — which **is inherited by child windows** — sidestepping the brief's
   "re-apply scale in every BeginChild" problem. Consequence: **size control applies to TTF
   faces only**; the Nexus-default face is unchanged (push nothing). No bitmap-scaling path.
4. **Tiny native unit-test harness** for the pure resolution layer (host g++, no MinGW/ImGui).

## Architecture

### 1. Pure resolution layer — `src/FontResolve.h` (header-only, no ImGui/Nexus)

```cpp
namespace AlterEgo::Font {
    enum class Face : int { NexusDefault = 0, Bundled = 1, Custom = 2 };

    struct Config {
        Face        face = Face::NexusDefault;
        std::string customPath;   // full path to a .ttf when face == Custom
        float       px   = 16.0f; // requested pixel size for TTF faces
    };

    // Stable Nexus identifier for a (face, px[, path]). Empty for the Nexus-default face
    // (no TTF). px is rounded to an int so 18.0 and 17.99 share one id.
    std::string Identifier(const Config& c);   // "" | "AlterEgo_F_BUNDLED_<px>" |
                                                //      "AlterEgo_F_C<fnv32(path)>_<px>"
    bool IsDefault(const Config& c);            // face == NexusDefault
}
```

Unit-tested in `tests/font_resolve_test.cpp`: empty id for default; bundled vs custom id
format; px rounding (18.0 and 17.99 → same id, 18 and 19 → different); distinct ids per
custom path (fnv32); `IsDefault`.

### 2. Font manager — `src/FontManager.{h,cpp}` (Nexus-bound)

Public surface:

```cpp
namespace AlterEgo::FontManager {
    void   Initialize(AddonAPI_t* api);     // store APIDefs; locate <addon>/fonts dir
    ImFont* ResolveDefault(const Font::Config& cfg); // ready TTF -> ImFont*; else nullptr
    void   Shutdown();                       // Fonts_Release everything
}
```

`ResolveDefault`:
- `cfg.face == NexusDefault` → return `nullptr` (caller pushes nothing).
- else lazily `RequestTtf(cfg)`; return the cached `ImFont*` if ready, else `nullptr`.

Async request discipline (mirrors Nexus async-texture rules):
- Cache `ImFont*` by `Identifier`. **Never assume a cached pointer is stable across frames** —
  Nexus re-fires the receive callback on every atlas rebuild with a NEW pointer; update the
  cache on every callback.
- **Never hold the manager mutex across `Fonts_AddFromFile`** — reserve bookkeeping under the
  lock, then unlock before the Nexus call (the callback may fire synchronously and re-enter).
- Re-request each frame until ready; an in-flight id is tracked so we don't double-request.
- The TTF path: bundled → the cached `<addon>/fonts/inter_regular.ttf` (only if present);
  custom → `cfg.customPath`.

### 3. Global application — `DefaultFontScope`

A single RAII guard near the top of the render callback (`AddonRender`), after any early-return
gate, plus the same in the achievement-tracker popout render:

```cpp
struct DefaultFontScope {
    bool pushed = false;
    explicit DefaultFontScope(const Font::Config& cfg) {
        ImFont* f = FontManager::ResolveDefault(cfg);
        if (f) { ImGui::PushFont(f); pushed = true; }
    }
    ~DefaultFontScope() { if (pushed) ImGui::PopFont(); }
};
```

Pushes **only a ready TTF**; for the Nexus-default face or a still-loading TTF it pushes
nothing (ambient font — identical to today). RAII balances the pop across all return paths.

### 4. Bundled font + custom folder

- `fonts/inter_regular.ttf` committed to the repo (one-time; TrueType `glyf` outlines, NOT
  `.otf`/CFF — stb_truetype can't parse CFF).
- On first selection of the **bundled** face (lazy, not at startup): download on a background
  thread via `HttpClient::DownloadToFile` from the repo raw URL
  (`https://raw.githubusercontent.com/PieOrCake/alter_ego/main/fonts/inter_regular.ttf`) to
  `<Paths_GetAddonDirectory("AlterEgo")>/fonts/inter_regular.ttf`. Validate: size > ~50 KB
  **and** TTF magic (first 4 bytes `00 01 00 00`, or `true`, or `ttcf`). Download to a
  `.part` file then `rename` into place (atomic). One attempt per session; on failure log a
  warning and fall back to the Nexus default. On startup, if a valid cached file already
  exists, mark it ready and skip the download.
- **Custom faces:** any `*.ttf` in `<addon>/fonts/` (folder created if missing). Selecting a
  file sets `face = Custom, customPath = <full path>`.
- **Dev-time note:** the raw URL resolves only once `fonts/inter_regular.ttf` is pushed to
  `main`. Until then the bundled download will 404 and fall back to the Nexus default — but
  the **entire feature is testable with a custom `.ttf`** (drop a file in `<addon>/fonts/`),
  so bundled-download validation is the one piece deferred until the asset is on `main`.

### 5. Settings UI (in `AddonOptions()`)

- A single **"Font"** dropdown: `Nexus default`, `Inter (bundled)`, then each `*.ttf` found in
  `<addon>/fonts/`. Selecting `Inter` sets `face = Bundled` (kicks off the download if not
  cached); selecting a file sets `face = Custom, customPath = …`.
- A **size slider** (10–32 px) shown **only when a TTF face is selected** (default 16).
- A hint line showing the `fonts/` path so users know where to drop their own faces.
- Each change writes `settings.json` via the existing `SaveSettings()`.

### 6. Config persistence

- `settings.json` gains a `"font"` object: `{ "face": int, "path": string, "px": float }`.
  Loaded in `LoadSettings()` with per-field type checks (matching the existing defensive
  loader); absent block → `{NexusDefault, "", 16}` (OOTB). No legacy migration (none exists).

## Files

| File | Change |
|------|--------|
| `src/FontResolve.h` | NEW — pure resolution layer |
| `src/FontManager.h/.cpp` | NEW — Nexus-bound font manager |
| `tests/font_resolve_test.cpp` | NEW — native unit test (compiled+run with host g++, e.g. `g++ -std=c++17 tests/font_resolve_test.cpp -o /tmp/font_test && /tmp/font_test`; not part of the MinGW DLL build) |
| `fonts/inter_regular.ttf` | NEW — bundled font asset |
| `src/dllmain.cpp` | settings struct + `LoadSettings`/`SaveSettings`; `AddonOptions()` UI; `DefaultFontScope` in `AddonRender` + popout; `FontManager::Initialize/Shutdown` in `AddonLoad/Unload` |
| `CMakeLists.txt` | add `FontManager.cpp`; (test build is separate/host-only) |

## Invariants

- **OOTB unchanged:** a fresh config (Nexus default) renders pixel-identical to today — the
  scope pushes nothing, widgets keep the ambient font.
- **Async-safe:** font appears within a frame or two of selection, no flicker; switching
  faces/sizes repeatedly never crashes (atlas rebuilds re-deliver pointers via the callback).
- **No deadlock:** the manager mutex is released before every `Fonts_AddFromFile` call.
- **Offline-safe:** with no network the bundled face silently falls back to the Nexus default;
  it retries next launch. Custom faces (local files) are unaffected.
- **Symbols unaffected:** no symbol/emoji glyphs expected from the TTF (Nexus atlas is ASCII-ish).
- **Build clean** after every task (`cd build && make -j$(nproc)`); native test passes.

## Out of scope (YAGNI)

- Per-window font overrides.
- Bitmap-font (FontUI/FontBig) scaling / S/M/L/XL presets.
- Sizing the Nexus-default face (pick a TTF to resize).
- Embedding the font in the DLL (we download instead, per decision).

## Task order (each ends green/committed)

1. Pure `FontResolve.h` + native unit test (host g++) — green.
2. Config: `font` slot in settings struct + `Load`/`SaveSettings` + defaults.
3. `FontManager` (Initialize/ResolveDefault/callback/Shutdown) wired into `AddonLoad/Unload`;
   build clean (no UI yet, default path only → OOTB unchanged).
4. Bundled-font download + disk cache + custom-folder scan.
5. Settings UI in `AddonOptions()` (dropdown + size slider + hint).
6. `DefaultFontScope` in `AddonRender` + the tracker popout.
7. In-game validation against the invariants.
