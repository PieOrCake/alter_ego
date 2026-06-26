# TrueType Font Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let the user pick a TrueType font, baked crisp at a chosen pixel size, applied globally to all AlterEgo text, replacing the blurry ambient ImGui default.

**Architecture:** A pure header-only resolution layer (`FontResolve.h`, unit-tested on the host) computes a stable Nexus font identifier from a `Font::Config`. A Nexus-bound `FontManager` loads TTFs asynchronously via `Fonts_AddFromFile` and caches the `ImFont*`. A single `DefaultFontScope` RAII at the top of `AddonRender` pushes the ready TTF (or nothing for the Nexus-default face). The bundled "Inter" face is downloaded on first use via the existing `HttpClient::DownloadToFile`.

**Tech Stack:** C++17, Dear ImGui 1.80, Nexus addon API, MinGW cross-compile (DLL). Pure layer additionally compiled + tested with host g++.

## Global Constraints

- Build the DLL after every task: `cd build && make -j$(nproc)` — must compile clean.
- Never deploy/copy the DLL into the game; the user deploys.
- **OOTB invariant:** a fresh config (Nexus default) renders pixel-identical to today — the scope pushes nothing.
- **Never hold the FontManager mutex across `Fonts_AddFromFile`** (the receive callback may re-enter).
- Nexus re-fires the receive callback on every atlas rebuild with a NEW `ImFont*`; update the cache on every callback; never assume a cached pointer is stable across frames.
- TTF does NOT unlock symbols/emoji (Nexus atlas is ASCII-ish) — no symbol expectations.
- Data/fonts dir is `AlterEgo::GW2API::GetDataDirectory() + "/fonts"` (= `<dll dir>/AlterEgo/fonts`).
- Bundled font URL: `https://raw.githubusercontent.com/PieOrCake/alter_ego/main/fonts/inter_regular.ttf`.
- Spec: [plans/2026-06-26-ttf-fonts-design.md](2026-06-26-ttf-fonts-design.md).

## File Structure

| File | Responsibility |
|------|----------------|
| `src/FontResolve.h` | NEW — pure: `Face`, `Config`, `Identifier`, `IsDefault`, `Fnv32`. No ImGui/Nexus. |
| `tests/font_resolve_test.cpp` | NEW — host-g++ unit test for `FontResolve.h`. |
| `src/FontManager.h` / `.cpp` | NEW — Nexus-bound async font loader + cache. |
| `fonts/inter_regular.ttf` | NEW — bundled font asset (committed; download source). |
| `src/dllmain.cpp` | font config global; `Load/SaveSettings`; `AddonOptions` UI; `DefaultFontScope` in `AddonRender`; `FontManager` init/shutdown in `AddonLoad/Unload`. |
| `CMakeLists.txt` | add `src/FontManager.cpp` to the DLL sources. |

---

### Task 1: Pure resolution layer + host unit test

**Files:**
- Create: `src/FontResolve.h`
- Create: `tests/font_resolve_test.cpp`

**Interfaces:**
- Produces: `AlterEgo::Font::Face {NexusDefault=0, Bundled=1, Custom=2}`; `struct Config { Face face; std::string customPath; float px; }`; `std::string Identifier(const Config&)`; `bool IsDefault(const Config&)`; `uint32_t Fnv32(const std::string&)`.

- [ ] **Step 1: Write the failing test** — `tests/font_resolve_test.cpp`:

```cpp
#include "../src/FontResolve.h"
#include <cstdio>
#include <cstdlib>
using namespace AlterEgo::Font;

static int g_fail = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); ++g_fail; } } while (0)

int main() {
    // Default face -> empty id, IsDefault true.
    Config d; // {NexusDefault, "", 16}
    CHECK(Identifier(d).empty());
    CHECK(IsDefault(d));

    // Bundled id format + px rounding.
    Config b{Face::Bundled, "", 16.0f};
    CHECK(Identifier(b) == "AlterEgo_F_BUNDLED_16");
    CHECK(!IsDefault(b));
    Config b18{Face::Bundled, "", 18.0f};
    Config b1799{Face::Bundled, "", 17.99f};
    CHECK(Identifier(b18) == Identifier(b1799));      // 18.0 and 17.99 -> same id
    Config b19{Face::Bundled, "", 19.0f};
    CHECK(Identifier(b18) != Identifier(b19));         // 18 vs 19 differ

    // Custom id depends on path; same path+px -> same id, different path -> different id.
    Config ca{Face::Custom, "/x/a.ttf", 16.0f};
    Config ca2{Face::Custom, "/x/a.ttf", 16.0f};
    Config cb{Face::Custom, "/x/b.ttf", 16.0f};
    CHECK(Identifier(ca) == Identifier(ca2));
    CHECK(Identifier(ca) != Identifier(cb));
    CHECK(Identifier(ca).rfind("AlterEgo_F_C", 0) == 0);

    if (g_fail == 0) { printf("ALL PASS\n"); return 0; }
    printf("%d FAILURE(S)\n", g_fail); return 1;
}
```

- [ ] **Step 2: Run it to confirm it fails (header missing)**

Run: `g++ -std=c++17 tests/font_resolve_test.cpp -o /tmp/font_test`
Expected: FAIL — `fatal error: ../src/FontResolve.h: No such file or directory`.

- [ ] **Step 3: Write `src/FontResolve.h`**

```cpp
#pragma once
// Pure font-resolution layer. NO ImGui / Nexus dependencies — header-only and
// host-unit-testable. Computes the stable Nexus font identifier for a config.
#include <string>
#include <cstdint>
#include <cmath>

namespace AlterEgo { namespace Font {

    enum class Face : int { NexusDefault = 0, Bundled = 1, Custom = 2 };

    struct Config {
        Face        face = Face::NexusDefault;
        std::string customPath;   // full path to a .ttf when face == Custom
        float       px   = 16.0f; // requested pixel size for TTF faces
    };

    inline bool IsDefault(const Config& c) { return c.face == Face::NexusDefault; }

    // 32-bit FNV-1a hash of a string (stable, for custom-path identifiers).
    inline uint32_t Fnv32(const std::string& s) {
        uint32_t h = 2166136261u;
        for (unsigned char ch : s) { h ^= ch; h *= 16777619u; }
        return h;
    }

    // Stable Nexus identifier. Empty for the Nexus-default face (no TTF). px is
    // rounded to the nearest int so 18.0 and 17.99 share one id (one baked atlas entry).
    inline std::string Identifier(const Config& c) {
        if (c.face == Face::NexusDefault) return std::string();
        int px = (int)std::lround(c.px);
        if (c.face == Face::Bundled)
            return "AlterEgo_F_BUNDLED_" + std::to_string(px);
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%08x", Fnv32(c.customPath));
        return std::string("AlterEgo_F_C") + buf + "_" + std::to_string(px);
    }

}} // namespace AlterEgo::Font
```

- [ ] **Step 4: Run the test to confirm it passes**

Run: `g++ -std=c++17 tests/font_resolve_test.cpp -o /tmp/font_test && /tmp/font_test`
Expected: `ALL PASS` (exit 0).

- [ ] **Step 5: Commit**

```bash
git add src/FontResolve.h tests/font_resolve_test.cpp
git commit -m "TTF fonts: pure resolution layer + host unit test"
```

---

### Task 2: Font config in settings (persist + load)

**Files:**
- Modify: `src/dllmain.cpp` — add include + global; `SaveSettings` (~line 1321); `LoadSettings` (~line 1369).

**Interfaces:**
- Consumes: `AlterEgo::Font::Config` (Task 1).
- Produces: global `AlterEgo::Font::Config g_FontConfig;` persisted under the `settings.json` key `"font"`.

- [ ] **Step 1: Include the header** near the other includes at the top of `src/dllmain.cpp`:

```cpp
#include "FontResolve.h"
```

- [ ] **Step 2: Add the global** next to the other settings globals (search for `static bool g_ShowQAIcon` and add below it):

```cpp
static AlterEgo::Font::Config g_FontConfig; // default = {NexusDefault, "", 16}
```

- [ ] **Step 3: Serialize in `SaveSettings`** — add before `std::ofstream file(path);` (~line 1338):

```cpp
    j["font"] = {
        {"face", (int)g_FontConfig.face},
        {"path", g_FontConfig.customPath},
        {"px",   g_FontConfig.px},
    };
```

- [ ] **Step 4: Deserialize in `LoadSettings`** — add before the closing `}` of `LoadSettings` (after the `getFloat("toast_pos_y", ...)` line, ~1368):

```cpp
    if (j.contains("font") && j["font"].is_object()) {
        const auto& f = j["font"];
        if (f.contains("face") && f["face"].is_number_integer()) {
            int face = f["face"].get<int>();
            if (face >= 0 && face <= 2) g_FontConfig.face = (AlterEgo::Font::Face)face;
        }
        if (f.contains("path") && f["path"].is_string()) g_FontConfig.customPath = f["path"].get<std::string>();
        if (f.contains("px")   && f["px"].is_number())    g_FontConfig.px = f["px"].get<float>();
    }
```

- [ ] **Step 5: Build** — `cd build && make -j$(nproc)`. Expect clean. Behaviour unchanged (config loaded but not yet used).

- [ ] **Step 6: Commit**

```bash
git add src/dllmain.cpp
git commit -m "TTF fonts: persist font config in settings.json"
```

---

### Task 3: FontManager (async load + cache) wired into load/unload

**Files:**
- Create: `src/FontManager.h`, `src/FontManager.cpp`
- Modify: `CMakeLists.txt` (add source); `src/dllmain.cpp` `AddonLoad` (~8868) + `AddonUnload` (~9016).

**Interfaces:**
- Consumes: `AlterEgo::Font::Config`, `Identifier`, `IsDefault` (Task 1); `AddonAPI_t* APIDefs`.
- Produces: `AlterEgo::FontManager::Initialize(AddonAPI_t*)`, `ImFont* ResolveDefault(const Font::Config&)`, `void Shutdown()`.

- [ ] **Step 1: Create `src/FontManager.h`**

```cpp
#pragma once
#include "FontResolve.h"
#include "nexus/Nexus.h"
struct ImFont;

namespace AlterEgo { namespace FontManager {
    void    Initialize(AddonAPI_t* api);
    // Ready TTF -> ImFont*; Nexus-default face or not-yet-loaded -> nullptr.
    ImFont* ResolveDefault(const Font::Config& cfg);
    void    Shutdown();
}}
```

- [ ] **Step 2: Create `src/FontManager.cpp`**

```cpp
#include "FontManager.h"
#include "GW2API.h"            // GetDataDirectory
#include "imgui.h"
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <thread>

namespace AlterEgo { namespace FontManager {

static AddonAPI_t*                          s_api = nullptr;
static std::string                          s_fontsDir;
static std::mutex                           s_mtx;
static std::unordered_map<std::string, ImFont*> s_fonts;     // id -> font (null until ready)
static std::unordered_set<std::string>      s_requested;     // ids in flight

// Forward decls for the bundled download (implemented in Task 4).
bool BundledFontReady();        // true if the cached inter_regular.ttf is present + valid
void EnsureBundledFont();       // kick a one-shot async download if not present

// Nexus receive callback: re-fires on every atlas rebuild with a NEW pointer.
static void ReceiveFont(const char* id, void* font) {
    std::lock_guard<std::mutex> lk(s_mtx);
    s_fonts[id] = (ImFont*)font;
}

void Initialize(AddonAPI_t* api) {
    s_api = api;
    s_fontsDir = AlterEgo::GW2API::GetDataDirectory() + "/fonts";
    std::error_code ec; std::filesystem::create_directories(s_fontsDir, ec);
}

static std::string BundledPath() { return s_fontsDir + "/inter_regular.ttf"; }

// Lazily request the TTF for a config. Returns the ImFont* if already loaded, else null.
static ImFont* RequestTtf(const Font::Config& cfg) {
    std::string id = Font::Identifier(cfg);
    if (id.empty() || !s_api || !s_api->Fonts_AddFromFile) return nullptr;

    std::string path;
    if (cfg.face == Font::Face::Bundled) {
        if (!BundledFontReady()) { EnsureBundledFont(); return nullptr; } // not downloaded yet
        path = BundledPath();
    } else {
        path = cfg.customPath;
    }
    if (path.empty()) return nullptr;

    std::unique_lock<std::mutex> lk(s_mtx);
    auto it = s_fonts.find(id);
    if (it != s_fonts.end() && it->second) return it->second; // ready
    if (s_requested.count(id)) return nullptr;                // in flight
    s_requested.insert(id);
    s_fonts[id] = nullptr;
    lk.unlock();                                              // MUST drop before Nexus call
    s_api->Fonts_AddFromFile(id.c_str(), cfg.px, path.c_str(), &ReceiveFont, nullptr);
    return nullptr;
}

ImFont* ResolveDefault(const Font::Config& cfg) {
    if (Font::IsDefault(cfg)) return nullptr;
    return RequestTtf(cfg);
}

void Shutdown() {
    std::lock_guard<std::mutex> lk(s_mtx);
    if (s_api && s_api->Fonts_Release)
        for (auto& kv : s_fonts) s_api->Fonts_Release(kv.first.c_str(), &ReceiveFont);
    s_fonts.clear();
    s_requested.clear();
    s_api = nullptr;
}

// ---- Bundled font: stubbed here, fully implemented in Task 4 ----
bool BundledFontReady() { return std::filesystem::exists(BundledPath()); }
void EnsureBundledFont() { /* Task 4 */ }

}} // namespace AlterEgo::FontManager
```

- [ ] **Step 3: Add the source to `CMakeLists.txt`** — in the `set(SOURCES ...)` list (which feeds `add_library(AlterEgo SHARED ${SOURCES})`), add a line `    src/FontManager.cpp` next to `    src/GW2API.cpp` (~line 44).

- [ ] **Step 4: Initialize in `AddonLoad`** — in `src/dllmain.cpp` after `APIDefs = aApi;` (~line 8868) and before the GUI registrations, add:

```cpp
    AlterEgo::FontManager::Initialize(aApi);
```
Add `#include "FontManager.h"` near the top includes.

- [ ] **Step 5: Shutdown in `AddonUnload`** — near the other deregistrations (~line 9056), add:

```cpp
    AlterEgo::FontManager::Shutdown();
```

- [ ] **Step 6: Build** — `cd build && make -j$(nproc)`. Expect clean. No behaviour change yet (nothing calls `ResolveDefault`).

- [ ] **Step 7: Commit**

```bash
git add src/FontManager.h src/FontManager.cpp src/dllmain.cpp CMakeLists.txt
git commit -m "TTF fonts: async FontManager wired into addon load/unload"
```

---

### Task 4: Bundled-font download + disk cache

**Files:**
- Modify: `src/FontManager.cpp` (replace the Task-3 stubs); ensure `#include "HttpClient.h"`.

**Interfaces:**
- Consumes: `HttpClient::DownloadToFile(const std::string& url, const std::string& path) -> bool`.
- Produces: working `BundledFontReady()` / `EnsureBundledFont()`.

- [ ] **Step 1: Add the include** at the top of `src/FontManager.cpp`:

```cpp
#include "HttpClient.h"
#include <atomic>
#include <fstream>
#include <cstdint>
```

- [ ] **Step 2: Replace the two stub functions** at the bottom of `src/FontManager.cpp` with:

```cpp
static std::atomic<bool> s_bundledFetching{false};

// Valid TTF = size > 50 KB AND magic in {00 01 00 00, 'true', 'ttcf'}.
static bool IsValidTtf(const std::string& path) {
    std::error_code ec;
    auto sz = std::filesystem::file_size(path, ec);
    if (ec || sz < 50u * 1024u) return false;
    std::ifstream f(path, std::ios::binary);
    unsigned char m[4] = {0,0,0,0};
    f.read((char*)m, 4);
    if (f.gcount() < 4) return false;
    bool sfnt = (m[0]==0x00 && m[1]==0x01 && m[2]==0x00 && m[3]==0x00);
    bool tru  = (m[0]=='t' && m[1]=='r' && m[2]=='u' && m[3]=='e');
    bool ttcf = (m[0]=='t' && m[1]=='t' && m[2]=='c' && m[3]=='f');
    return sfnt || tru || ttcf;
}

bool BundledFontReady() { return IsValidTtf(BundledPath()); }

void EnsureBundledFont() {
    if (BundledFontReady()) return;
    if (s_bundledFetching.exchange(true)) return;     // one attempt per session
    std::string dir = s_fontsDir;
    std::thread([dir]() {
        const std::string url  = "https://raw.githubusercontent.com/PieOrCake/alter_ego/main/fonts/inter_regular.ttf";
        const std::string dst  = dir + "/inter_regular.ttf";
        const std::string part = dst + ".part";
        if (HttpClient::DownloadToFile(url, part) && IsValidTtf(part)) {
            std::error_code ec;
            std::filesystem::rename(part, dst, ec);    // atomic move into place
            if (ec) std::filesystem::remove(part, ec);
        } else {
            std::error_code ec; std::filesystem::remove(part, ec);
            if (APIDefs && APIDefs->Log)
                APIDefs->Log(LOGL_WARNING, "AlterEgo", "Bundled font download failed; using Nexus default.");
        }
        s_bundledFetching = false;
    }).detach();
}
```

NOTE: `APIDefs` is a non-static global in `dllmain.cpp`. Add `extern AddonAPI_t* APIDefs;` near the top of `FontManager.cpp` (same local-extern pattern as `Commerce.cpp`/`WikiImage.cpp`; `LOGL_WARNING` is the Nexus `ELogLevel` value, confirmed in `include/nexus/Nexus.h`).

- [ ] **Step 3: Build** — `cd build && make -j$(nproc)`. Expect clean.

- [ ] **Step 4: Commit**

```bash
git add src/FontManager.cpp
git commit -m "TTF fonts: bundled-font download with validation + atomic cache"
```

---

### Task 5: Settings UI — Font dropdown + size slider

**Files:**
- Modify: `src/dllmain.cpp` — `AddonOptions()` (~line 13725); add a small `*.ttf` folder scan helper.

**Interfaces:**
- Consumes: `g_FontConfig`, `SaveSettings`, `AlterEgo::GW2API::GetDataDirectory`.

- [ ] **Step 1: Add a folder-scan helper** just above `void AddonOptions()`:

```cpp
// List *.ttf filenames in <data>/fonts (created if missing). Returns bare filenames.
static std::vector<std::string> ScanFontFolder(std::string& outDir) {
    outDir = AlterEgo::GW2API::GetDataDirectory() + "/fonts";
    std::error_code ec; std::filesystem::create_directories(outDir, ec);
    std::vector<std::string> out;
    for (auto& e : std::filesystem::directory_iterator(outDir, ec)) {
        if (!e.is_regular_file()) continue;
        std::string name = e.path().filename().string();
        if (name.size() > 4) {
            std::string ext = name.substr(name.size() - 4);
            for (auto& ch : ext) ch = (char)tolower((unsigned char)ch);
            if (ext == ".ttf") out.push_back(name);
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}
```

- [ ] **Step 2: Add the Font UI** inside `AddonOptions()`, after the existing options (before its closing `}`):

```cpp
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.85f, 0.78f, 0.55f, 1.0f), "Font");

    std::string fontsDir;
    std::vector<std::string> ttfs = ScanFontFolder(fontsDir);

    // Current selection label.
    std::string current;
    if (g_FontConfig.face == AlterEgo::Font::Face::NexusDefault) current = "Nexus default";
    else if (g_FontConfig.face == AlterEgo::Font::Face::Bundled)  current = "Inter (bundled)";
    else {
        current = g_FontConfig.customPath;
        auto slash = current.find_last_of("/\\");
        if (slash != std::string::npos) current = current.substr(slash + 1);
    }

    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::BeginCombo("##fontface", current.c_str())) {
        if (ImGui::Selectable("Nexus default", g_FontConfig.face == AlterEgo::Font::Face::NexusDefault)) {
            g_FontConfig.face = AlterEgo::Font::Face::NexusDefault;
            g_FontConfig.customPath.clear();
            SaveSettings();
        }
        if (ImGui::Selectable("Inter (bundled)", g_FontConfig.face == AlterEgo::Font::Face::Bundled)) {
            g_FontConfig.face = AlterEgo::Font::Face::Bundled;
            g_FontConfig.customPath.clear();
            SaveSettings();
        }
        for (const auto& name : ttfs) {
            std::string full = fontsDir + "/" + name;
            bool sel = (g_FontConfig.face == AlterEgo::Font::Face::Custom && g_FontConfig.customPath == full);
            if (ImGui::Selectable(name.c_str(), sel)) {
                g_FontConfig.face = AlterEgo::Font::Face::Custom;
                g_FontConfig.customPath = full;
                SaveSettings();
            }
        }
        ImGui::EndCombo();
    }

    // Size slider only for TTF faces (the Nexus-default face is unscaled).
    if (g_FontConfig.face != AlterEgo::Font::Face::NexusDefault) {
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::SliderFloat("Size (px)", &g_FontConfig.px, 10.0f, 32.0f, "%.0f"))
            ; // committed on release below
        if (ImGui::IsItemDeactivatedAfterEdit()) SaveSettings();
    }

    ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f),
                       "Drop your own .ttf files in: %s", fontsDir.c_str());
```

- [ ] **Step 3: Build** — `cd build && make -j$(nproc)`. Expect clean.

- [ ] **Step 4: In-game check** — open AlterEgo options; the Font dropdown lists Nexus default / Inter / any dropped `.ttf`; the size slider shows for TTF faces; selections persist across reload (`settings.json` has the `font` block). The font is not yet applied to text (Task 6).

- [ ] **Step 5: Commit**

```bash
git add src/dllmain.cpp
git commit -m "TTF fonts: settings UI (font dropdown + size slider)"
```

---

### Task 6: Apply the font globally via DefaultFontScope

**Files:**
- Modify: `src/dllmain.cpp` — `AddonRender()` (~line 12474).

**Interfaces:**
- Consumes: `AlterEgo::FontManager::ResolveDefault`, `g_FontConfig`.

- [ ] **Step 1: Add the RAII scope type** just above `void AddonRender()`:

```cpp
// Pushes the configured default TTF for the whole frame (all windows + popouts).
// Pushes NOTHING for the Nexus-default face or a still-loading TTF (OOTB invariant).
struct DefaultFontScope {
    bool pushed = false;
    explicit DefaultFontScope(const AlterEgo::Font::Config& cfg) {
        ImFont* f = AlterEgo::FontManager::ResolveDefault(cfg);
        if (f) { ImGui::PushFont(f); pushed = true; }
    }
    ~DefaultFontScope() { if (pushed) ImGui::PopFont(); }
};
```

- [ ] **Step 2: Instantiate it at the top of `AddonRender`** — as the first statement inside `void AddonRender() {`, before the `IconManager::Tick()` line:

```cpp
    DefaultFontScope _fontScope(g_FontConfig);
```

(RAII spans the whole function, so every window AlterEgo draws this frame — main window and popouts — inherits the font via `PushFont`, and the pop runs on every return path.)

- [ ] **Step 3: Build** — `cd build && make -j$(nproc)`. Expect clean.

- [ ] **Step 4: Commit**

```bash
git add src/dllmain.cpp
git commit -m "TTF fonts: apply the configured font globally in AddonRender"
```

---

### Task 7: In-game validation

**Files:** none (validation only). Provide a real `.ttf` for the custom path (bundled download only works once the asset is on `main`).

- [ ] **Step 1: OOTB** — fresh/`Nexus default` config renders identical to the pre-feature build (no font change anywhere).
- [ ] **Step 2: Custom TTF** — drop a `.ttf` into `<data>/fonts`, pick it, set size 20 → all AlterEgo text (tabs, build cards, achievements, tracker popout) renders in that face, crisp; size changes re-bake crisply within a frame or two; no flicker, no crash when switching faces/sizes repeatedly.
- [ ] **Step 3: Bundled (if asset on main)** — pick `Inter (bundled)`; it downloads once to `<data>/fonts/inter_regular.ttf`, then applies; offline → silently falls back to Nexus default and retries next launch.
- [ ] **Step 4: Persistence** — choice + size survive a reload (`/v2`-independent; stored in `settings.json`).
- [ ] **Step 5: Update handover.md** with the finished state; commit.

```bash
git add handover.md
git commit -m "TTF fonts: finish + update handover"
```

## Self-Review

- **Spec coverage:** pure layer + tests (T1); config persistence (T2); FontManager async + unlock-before-Nexus + callback re-delivery (T3); bundled download + validation + atomic cache + offline fallback (T4); settings dropdown + size slider + folder hint (T5); global `DefaultFontScope` / OOTB / PushFont-inheritance (T6); invariants validated (T7). All design sections mapped.
- **Placeholder scan:** none. The Task-3 bundled stubs are explicitly replaced in Task 4 (noted in both). The optional `Log` line has a documented fallback.
- **Type consistency:** `Font::Config`/`Face`/`Identifier`/`IsDefault`/`Fnv32` (T1) used verbatim in T2/T3/T5/T6; `FontManager::Initialize/ResolveDefault/Shutdown` (T3) used in T6; `BundledFontReady`/`EnsureBundledFont` declared in T3, defined in T4; `g_FontConfig` consistent across T2/T5/T6; `DownloadToFile` signature matches `HttpClient.h`.
- **Known risk:** `APIDefs->Log` / `ELogLevel_WARNING` naming depends on the vendored Nexus header — Task 4 notes dropping the log line if it doesn't match.
