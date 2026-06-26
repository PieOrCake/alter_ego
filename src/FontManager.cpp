#include "FontManager.h"
#include "GW2API.h"            // GetDataDirectory
#include "HttpClient.h"
#include "imgui.h"
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <thread>
#include <atomic>
#include <fstream>
#include <cstdint>

// Non-static global from dllmain.cpp (same local-extern pattern as Commerce/WikiImage).
extern AddonAPI_t* APIDefs;

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

// ---- Bundled font: download on first use, validate, cache atomically ----
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
        if (Skinventory::HttpClient::DownloadToFile(url, part) && IsValidTtf(part)) {
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

}} // namespace AlterEgo::FontManager
