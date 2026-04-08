#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <set>
#include <mutex>
#include <thread>
#include <fstream>
#include <filesystem>
#include <chrono>

#include "nexus/Nexus.h"
#include "imgui.h"
#include "GW2API.h"
#include "IconManager.h"
#include "ChatLink.h"
#include "HoardAndSeekAPI.h"
#include "SpecDescriptions.h"
#include "SkinCache.h"
#include "OwnedSkins.h"
#include "Commerce.h"
#include "WikiImage.h"
#include "HttpClient.h"
#include <nlohmann/json.hpp>

// Version constants
#define V_MAJOR 0
#define V_MINOR 9
#define V_BUILD 0
#define V_REVISION 0

// Quick Access icon identifiers
#define QA_ID "QA_ALTER_EGO"
#define TEX_ICON "TEX_ALTER_EGO_ICON"
#define TEX_ICON_HOVER "TEX_ALTER_EGO_ICON_HOVER"

// Embedded 32x32 silhouette icon (normal - light gray)
static const unsigned char ICON_NORMAL[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a,
    0xf4, 0x00, 0x00, 0x00, 0x6a, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0xed, 0xd5, 0xb1, 0x0d, 0x00,
    0x20, 0x0c, 0x04, 0xc1, 0xa3, 0x40, 0x42, 0x90, 0x10, 0xa1, 0x50, 0x52, 0x44, 0x89, 0x48, 0x88,
    0x6c, 0x6f, 0x87, 0xeb, 0xaf, 0x78, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x60, 0xb7, 0x07, 0x39, 0xa0, 0x00, 0x01, 0xd0, 0x5e, 0xa0, 0xb4, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};
static const unsigned int ICON_NORMAL_size = 154;

// Embedded 32x32 silhouette icon (hover - white)
static const unsigned char ICON_HOVER[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a,
    0xf4, 0x00, 0x00, 0x00, 0x6a, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0xed, 0xd5, 0xb1, 0x0d, 0x00,
    0x20, 0x0c, 0x04, 0xc1, 0xa3, 0x40, 0x42, 0x90, 0x10, 0xa1, 0x50, 0x52, 0x44, 0x89, 0x48, 0x88,
    0x6c, 0x6f, 0x87, 0xeb, 0xaf, 0x78, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x60, 0xb7, 0x07, 0x39, 0xa0, 0x00, 0x01, 0xd0, 0x5e, 0xa0, 0xb4, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};
static const unsigned int ICON_HOVER_size = 154;

// Global variables
HMODULE hSelf;
AddonDefinition_t AddonDef{};
AddonAPI_t* APIDefs = nullptr;
bool g_WindowVisible = false;

// UI State
static int g_SelectedCharIdx = -1;
static int g_SelectedTab = 0;          // 0 = Equipment, 1 = Build
static int g_SelectedEquipTab = 0;     // Equipment tab filter
static int g_SelectedBuildTab = 0;     // Build tab filter
static bool g_ShowQAIcon = true;
static bool g_CompactCharList = false;
static bool g_ShowCraftingIcons = true;
static bool g_ShowAge = false;
static bool g_ShowPlaytime = false;
static bool g_ShowLastLogin = false;
static int g_BirthdayMode = 1;         // 0 = Always, 1 = A week out, 2 = Never
static bool g_DetailsFetched = false;  // Track if we've requested details for current character
static int g_MainTab = 0;             // 0 = Characters, 1 = Build Library, 2 = Skinventory
static std::chrono::steady_clock::time_point g_FetchDoneTime{}; // For status message fade-out

// Skinventory UI state
static int g_SkinActiveTab = 0; // 0 = Browser, 1 = Shopping List
static bool g_SkinSwitchToBrowser = false;
static bool g_SkinScrollToSkin = false;
static std::string g_SkinSelectedType = "Armor";
static std::string g_SkinSelectedWeightClass = "Heavy";
static std::string g_SkinSelectedSubtype = "Helm";
static uint32_t g_SkinSelectedId = 0;
static char g_SkinSearchFilter[256] = "";
static bool g_SkinShowOwned = true;
static bool g_SkinShowUnowned = true;
static bool g_SkinRefreshOwned = false;
static bool g_SkinInitialized = false;

// Shopping list state
struct SkinShopEntry {
    uint32_t skinId;
    int price;      // in copper
    int source;     // 0 = TP, 1 = Vendor
};
static std::vector<SkinShopEntry> g_SkinShopList;
static bool g_SkinShopListDirty = true;

// Clears tracker state
#define EV_AE_CLEARS_ACH_RESPONSE "EV_ALTER_EGO_CLEARS_ACH_RESP"
static void OnClearsAchResponse(void* eventArgs);

// Achievement categories: 88 = Daily Fractals, 475 = Daily Raid Bounties, 477 = Weekly Raids
static const uint32_t CAT_DAILY_FRACTALS = 88;
static const uint32_t CAT_DAILY_BOUNTIES = 475;
static const uint32_t CAT_WEEKLY_RAIDS   = 477;
static const uint32_t ACH_WEEKLY_STRIKES = 9125; // "Weekly Raid Encounters" — tracks all strikes

struct ClearEntry {
    uint32_t id = 0;
    std::string name;
    std::string tier;              // For fractals: "T1".."T4", "Rec"
    bool done = false;
    int32_t current = 0;
    int32_t max = 0;
    std::vector<std::string> bitNames;  // From API bits[].text
    std::vector<bool> bitDone;          // Per-bit completion from H&S
};

static std::vector<ClearEntry> g_DailyFractals;   // cat 88
static std::vector<ClearEntry> g_DailyBounties;   // cat 475
static std::vector<ClearEntry> g_WeeklyWings;     // cat 477 per-wing achievements
static ClearEntry g_WeeklyStrikes;                 // ach 9125

static std::mutex g_ClearsMutex;
static bool g_ClearsFetching = false;
static bool g_ClearsFetched = false;
static std::string g_ClearsStatusMsg;

// Reset time tracking
static std::chrono::system_clock::time_point g_LastDailyReset{};
static std::chrono::system_clock::time_point g_LastWeeklyReset{};
static std::chrono::steady_clock::time_point g_LastClearsCompletionQuery{};

// Character search
static char g_CharSearchBuf[128] = "";

// MumbleLink identity for current character indicator
struct MumbleIdentity {
    char Name[20];
    unsigned Profession;
    unsigned Specialization;
    unsigned Race;
    unsigned MapID;
    unsigned WorldID;
    unsigned TeamColorID;
    bool IsCommander;
    float FOV;
    unsigned UISize;
};
static std::string g_CurrentCharName; // currently logged-in character name from MumbleLink

// Build Library UI state
static int g_LibSelectedIdx = -1;
static int g_LibFilterMode = 0;        // 0=All, 1=PvE, 2=WvW, 3=PvP, 4=Raid, 5=Fractal
static char g_LibSearchBuf[128] = "";
static char g_LibImportBuf[512] = "";
static char g_LibImportName[128] = "";
static int g_LibImportMode = 0;        // GameMode for import
static bool g_LibShowImport = false;
static std::string g_LibImportError;
static bool g_LibDetailsFetched = false;
static int g_LibDragIdx = -1;          // drag-and-drop source index
static char g_LibEditName[128] = "";   // inline rename buffer
static char g_LibEditNotes[512] = ""; // inline notes buffer
static std::string g_LibEditBuildId;   // which build is being edited

// Character list sorting
enum CharSortMode { Sort_Custom = 0, Sort_Name, Sort_Profession, Sort_Level, Sort_Age, Sort_Birthday };
static int g_CharSortMode = Sort_Custom;
static bool g_CharSortAscending = true;            // true = default direction, false = reversed
static std::vector<int> g_CharDisplayOrder;        // indices into GetCharacters()
static std::vector<std::string> g_CustomCharOrder;  // saved custom name order
static int g_CharDragIdx = -1;                      // drag-and-drop source index in display order
static size_t g_LastCharCount = 0;                  // detect character list changes

static void SaveCharSortConfig();
static void LoadCharSortConfig();
static int DaysUntilBirthday(const std::string& created);

static void RebuildCharDisplayOrder() {
    const auto& chars = AlterEgo::GW2API::GetCharacters();
    int n = (int)chars.size();
    g_CharDisplayOrder.resize(n);
    for (int i = 0; i < n; i++) g_CharDisplayOrder[i] = i;

    bool asc = g_CharSortAscending;
    switch (g_CharSortMode) {
        case Sort_Name:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    return asc ? chars[a].name < chars[b].name
                               : chars[a].name > chars[b].name;
                });
            break;
        case Sort_Profession:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    if (chars[a].profession != chars[b].profession)
                        return asc ? chars[a].profession < chars[b].profession
                                   : chars[a].profession > chars[b].profession;
                    return chars[a].name < chars[b].name;
                });
            break;
        case Sort_Level:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    if (chars[a].level != chars[b].level)
                        return asc ? chars[a].level > chars[b].level   // asc = highest first
                                   : chars[a].level < chars[b].level;
                    return chars[a].name < chars[b].name;
                });
            break;
        case Sort_Age:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    return asc ? chars[a].created < chars[b].created   // asc = oldest first
                               : chars[a].created > chars[b].created;
                });
            break;
        case Sort_Birthday:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    int da = DaysUntilBirthday(chars[a].created);
                    int db = DaysUntilBirthday(chars[b].created);
                    if (da < 0) da = 999; // unparseable dates go to end
                    if (db < 0) db = 999;
                    if (da != db)
                        return asc ? da < db : da > db;  // asc = soonest first
                    return chars[a].name < chars[b].name;
                });
            break;
        case Sort_Custom:
        default: {
            // Order by g_CustomCharOrder, unknowns appended at end
            std::unordered_map<std::string, int> nameToIdx;
            for (int i = 0; i < n; i++) nameToIdx[chars[i].name] = i;

            g_CharDisplayOrder.clear();
            // First: characters in custom order
            for (const auto& name : g_CustomCharOrder) {
                auto it = nameToIdx.find(name);
                if (it != nameToIdx.end()) {
                    g_CharDisplayOrder.push_back(it->second);
                    nameToIdx.erase(it);
                }
            }
            // Then: any new characters not yet in custom order
            for (int i = 0; i < n; i++) {
                if (nameToIdx.find(chars[i].name) != nameToIdx.end()) {
                    g_CharDisplayOrder.push_back(i);
                }
            }
            break;
        }
    }
    g_LastCharCount = chars.size();
}

// Character refresh selection popup state
static bool g_RefreshPopupOpen = false;
static bool g_RefreshListFetching = false;     // waiting for char list from H&S
static std::vector<std::string> g_RefreshNames; // all character names from API
static std::vector<bool> g_RefreshSelection;    // parallel checkbox state

// Gear customization dialog state
static bool g_GearDialogOpen = false;
static std::string g_GearDialogSlot;          // Which slot is being edited
static std::string g_GearDialogBuildId;       // Which saved build
static char g_GearStatSearch[128] = "";       // Search filter for stat combos
static uint32_t g_GearSelectedStatId = 0;     // Currently selected stat in dialog
static int g_GearSelectorTab = 0;             // 0 = Stats, 1 = Rune/Sigil

// Compute days until next character birthday from ISO date string (e.g. "2013-06-25T...")
// Returns -1 if the date can't be parsed. Returns 0 on the birthday itself.
static int DaysUntilBirthday(const std::string& created) {
    if (created.size() < 10) return -1;
    int year, month, day;
    if (sscanf(created.c_str(), "%d-%d-%d", &year, &month, &day) != 3) return -1;

    time_t now = std::time(nullptr);
    struct tm* today = std::localtime(&now);

    // Build this year's birthday
    struct tm bday = {};
    bday.tm_year = today->tm_year;
    bday.tm_mon = month - 1;
    bday.tm_mday = day;
    bday.tm_hour = 0;
    time_t bdayTime = std::mktime(&bday);

    // Build today at midnight for clean day diff
    struct tm todayMid = *today;
    todayMid.tm_hour = 0;
    todayMid.tm_min = 0;
    todayMid.tm_sec = 0;
    time_t todayTime = std::mktime(&todayMid);

    int diff = (int)std::difftime(bdayTime, todayTime) / 86400;
    if (diff < 0) {
        // Birthday already passed this year — next year
        bday.tm_year = today->tm_year + 1;
        bdayTime = std::mktime(&bday);
        diff = (int)std::difftime(bdayTime, todayTime) / 86400;
    }
    return diff;
}

// Compute character age in years from ISO date string
static int CharacterAgeYears(const std::string& created) {
    if (created.size() < 10) return -1;
    int year, month, day;
    if (sscanf(created.c_str(), "%d-%d-%d", &year, &month, &day) != 3) return -1;

    time_t now = std::time(nullptr);
    struct tm* today = std::localtime(&now);
    int age = (today->tm_year + 1900) - year;
    // Haven't had birthday yet this year
    if ((today->tm_mon + 1) < month || ((today->tm_mon + 1) == month && today->tm_mday < day))
        age--;
    return age;
}

// Strip GW2 markup tags from description text
// Converts <br> to newline, removes <c=@...> and </c> tags
static std::string StripGW2Markup(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '<') {
            // Find closing >
            size_t end = text.find('>', i);
            if (end != std::string::npos) {
                std::string tag = text.substr(i, end - i + 1);
                // Convert <br> to newline
                if (tag == "<br>" || tag == "<br/>" || tag == "<br />") {
                    result += '\n';
                }
                // Skip all other tags (<c=@...>, </c>, etc.)
                i = end + 1;
                continue;
            }
        }
        result += text[i];
        i++;
    }
    return result;
}

// GW2 API weapon type → current in-game display name
static const char* WeaponDisplayName(const std::string& apiName) {
    if (apiName == "Harpoon") return "Spear";
    if (apiName == "Speargun") return "Harpoon Gun";
    return apiName.c_str();
}

// GW2 rarity colors
static ImVec4 GetRarityColor(const std::string& rarity) {
    if (rarity == "Legendary")  return ImVec4(0.63f, 0.39f, 0.78f, 1.0f);
    if (rarity == "Ascended")   return ImVec4(0.90f, 0.39f, 0.55f, 1.0f);
    if (rarity == "Exotic")     return ImVec4(1.00f, 0.65f, 0.00f, 1.0f);
    if (rarity == "Rare")       return ImVec4(1.00f, 0.86f, 0.20f, 1.0f);
    if (rarity == "Masterwork") return ImVec4(0.12f, 0.71f, 0.12f, 1.0f);
    if (rarity == "Fine")       return ImVec4(0.35f, 0.63f, 0.90f, 1.0f);
    return ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
}

// GW2 profession colors
static ImVec4 GetProfessionColor(const std::string& prof) {
    if (prof == "Guardian")     return ImVec4(0.47f, 0.71f, 0.86f, 1.0f);
    if (prof == "Warrior")      return ImVec4(1.00f, 0.83f, 0.22f, 1.0f);
    if (prof == "Engineer")     return ImVec4(0.85f, 0.56f, 0.25f, 1.0f);
    if (prof == "Ranger")       return ImVec4(0.55f, 0.80f, 0.28f, 1.0f);
    if (prof == "Thief")        return ImVec4(0.78f, 0.31f, 0.31f, 1.0f);
    if (prof == "Elementalist") return ImVec4(0.94f, 0.42f, 0.42f, 1.0f);
    if (prof == "Mesmer")       return ImVec4(0.71f, 0.31f, 0.71f, 1.0f);
    if (prof == "Necromancer")  return ImVec4(0.32f, 0.75f, 0.54f, 1.0f);
    if (prof == "Revenant")     return ImVec4(0.63f, 0.22f, 0.22f, 1.0f);
    return ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
}

// Core profession icon URL from API (fetched at startup via FetchProfessionInfoAsync)
static const char* GetProfessionIconUrl(const std::string& prof) {
    const auto* info = AlterEgo::GW2API::GetProfessionInfo(prof);
    if (info && !info->icon_url.empty()) return info->icon_url.c_str();
    return nullptr;
}

// Stable icon ID for profession icons (offset to avoid collision with item/skill IDs)
static uint32_t GetProfessionIconId(const std::string& prof) {
    static const std::unordered_map<std::string, uint32_t> ids = {
        {"Guardian", 9000001}, {"Warrior", 9000002}, {"Engineer", 9000003},
        {"Ranger", 9000004}, {"Thief", 9000005}, {"Elementalist", 9000006},
        {"Mesmer", 9000007}, {"Necromancer", 9000008}, {"Revenant", 9000009}
    };
    auto it = ids.find(prof);
    return it != ids.end() ? it->second : 0;
}

// Crafting discipline icon helpers (wiki tango icons, stable IDs 9100001+)
static uint32_t GetCraftingIconId(const std::string& disc) {
    static const std::unordered_map<std::string, uint32_t> ids = {
        {"Armorsmith", 9100001}, {"Artificer", 9100002}, {"Chef", 9100003},
        {"Huntsman", 9100004}, {"Jeweler", 9100005}, {"Leatherworker", 9100006},
        {"Scribe", 9100007}, {"Tailor", 9100008}, {"Weaponsmith", 9100009}
    };
    auto it = ids.find(disc);
    return it != ids.end() ? it->second : 0;
}

static const char* GetCraftingIconUrl(const std::string& disc) {
    static const std::unordered_map<std::string, std::string> urls = {
        {"Armorsmith",    "https://wiki.guildwars2.com/images/3/32/Armorsmith_tango_icon_20px.png"},
        {"Artificer",     "https://wiki.guildwars2.com/images/b/b7/Artificer_tango_icon_20px.png"},
        {"Chef",          "https://wiki.guildwars2.com/images/8/8f/Chef_tango_icon_20px.png"},
        {"Huntsman",      "https://wiki.guildwars2.com/images/f/f3/Huntsman_tango_icon_20px.png"},
        {"Jeweler",       "https://wiki.guildwars2.com/images/f/f2/Jeweler_tango_icon_20px.png"},
        {"Leatherworker", "https://wiki.guildwars2.com/images/e/e5/Leatherworker_tango_icon_20px.png"},
        {"Scribe",        "https://wiki.guildwars2.com/images/0/0b/Scribe_tango_icon_20px.png"},
        {"Tailor",        "https://wiki.guildwars2.com/images/4/4d/Tailor_tango_icon_20px.png"},
        {"Weaponsmith",   "https://wiki.guildwars2.com/images/4/46/Weaponsmith_tango_icon_20px.png"}
    };
    auto it = urls.find(disc);
    return it != urls.end() ? it->second.c_str() : nullptr;
}

// Copy text to Windows clipboard
static void CopyToClipboard(const std::string& text) {
    if (!OpenClipboard(NULL)) return;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem) {
        char* p = (char*)GlobalLock(hMem);
        if (p) {
            memcpy(p, text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
    }
    CloseClipboard();
}

// Settings persistence
static void SaveSettings() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/settings.json";
    nlohmann::json j;
    j["show_qa_icon"] = g_ShowQAIcon;
    j["compact_char_list"] = g_CompactCharList;
    j["show_crafting_icons"] = g_ShowCraftingIcons;
    j["show_age"] = g_ShowAge;
    j["show_playtime"] = g_ShowPlaytime;
    j["show_last_login"] = g_ShowLastLogin;
    j["birthday_mode"] = g_BirthdayMode;
    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadSettings() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/settings.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        if (j.contains("show_qa_icon")) g_ShowQAIcon = j["show_qa_icon"].get<bool>();
        if (j.contains("compact_char_list")) g_CompactCharList = j["compact_char_list"].get<bool>();
        if (j.contains("show_crafting_icons")) g_ShowCraftingIcons = j["show_crafting_icons"].get<bool>();
        if (j.contains("show_age")) g_ShowAge = j["show_age"].get<bool>();
        if (j.contains("show_playtime")) g_ShowPlaytime = j["show_playtime"].get<bool>();
        if (j.contains("show_last_login")) g_ShowLastLogin = j["show_last_login"].get<bool>();
        if (j.contains("birthday_mode")) g_BirthdayMode = j["birthday_mode"].get<int>();
    } catch (...) {}
}

// Character sort persistence
static void SaveCharSortConfig() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/char_sort.json";
    nlohmann::json j;
    j["sort_mode"] = g_CharSortMode;
    j["sort_ascending"] = g_CharSortAscending;
    j["custom_order"] = g_CustomCharOrder;
    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadCharSortConfig() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/char_sort.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        if (j.contains("sort_mode")) g_CharSortMode = j["sort_mode"].get<int>();
        if (j.contains("sort_ascending")) g_CharSortAscending = j["sort_ascending"].get<bool>();
        if (j.contains("custom_order")) {
            g_CustomCharOrder.clear();
            for (const auto& name : j["custom_order"])
                g_CustomCharOrder.push_back(name.get<std::string>());
        }
    } catch (...) {}
}

// Clears cache persistence (forward declarations for functions defined in Clears backend section)
static std::chrono::system_clock::time_point CalcLastDailyReset(std::chrono::system_clock::time_point now);
static std::chrono::system_clock::time_point CalcLastWeeklyReset(std::chrono::system_clock::time_point now);
static void FetchClears();

static void SaveClearsCache() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/clears_cache.json";

    nlohmann::json j;
    // Store fetch timestamp as seconds since epoch
    auto epochSecs = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    j["fetch_time"] = epochSecs;

    auto serializeEntry = [](const ClearEntry& e) -> nlohmann::json {
        nlohmann::json ej;
        ej["id"] = e.id;
        ej["name"] = e.name;
        ej["tier"] = e.tier;
        ej["done"] = e.done;
        ej["current"] = e.current;
        ej["max"] = e.max;
        ej["bitNames"] = e.bitNames;
        std::vector<int> bits;
        for (bool b : e.bitDone) bits.push_back(b ? 1 : 0);
        ej["bitDone"] = bits;
        return ej;
    };

    nlohmann::json fractals = nlohmann::json::array();
    for (const auto& e : g_DailyFractals) fractals.push_back(serializeEntry(e));
    j["dailyFractals"] = fractals;

    nlohmann::json bounties = nlohmann::json::array();
    for (const auto& e : g_DailyBounties) bounties.push_back(serializeEntry(e));
    j["dailyBounties"] = bounties;

    nlohmann::json wings = nlohmann::json::array();
    for (const auto& e : g_WeeklyWings) wings.push_back(serializeEntry(e));
    j["weeklyWings"] = wings;

    j["weeklyStrikes"] = serializeEntry(g_WeeklyStrikes);

    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadClearsCache() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/clears_cache.json";
    std::ifstream file(path);
    if (!file.is_open()) return;

    auto deserializeEntry = [](const nlohmann::json& ej) -> ClearEntry {
        ClearEntry e;
        e.id = ej.value("id", 0u);
        e.name = ej.value("name", "");
        e.tier = ej.value("tier", "");
        e.done = ej.value("done", false);
        e.current = ej.value("current", 0);
        e.max = ej.value("max", 0);
        if (ej.contains("bitNames") && ej["bitNames"].is_array()) {
            for (const auto& bn : ej["bitNames"]) e.bitNames.push_back(bn.get<std::string>());
        }
        if (ej.contains("bitDone") && ej["bitDone"].is_array()) {
            for (const auto& bd : ej["bitDone"]) e.bitDone.push_back(bd.get<int>() != 0);
        }
        return e;
    };

    try {
        auto j = nlohmann::json::parse(file);

        int64_t fetchEpoch = j.value("fetch_time", (int64_t)0);
        auto fetchTime = std::chrono::system_clock::from_time_t((time_t)fetchEpoch);

        // Check if a reset has occurred since the cached data was fetched
        auto now = std::chrono::system_clock::now();
        auto dailyReset = CalcLastDailyReset(now);
        auto weeklyReset = CalcLastWeeklyReset(now);
        bool dailyStale = fetchTime < dailyReset;
        bool weeklyStale = fetchTime < weeklyReset;

        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);

            // Load daily data only if no daily reset has occurred since fetch
            if (!dailyStale) {
                if (j.contains("dailyFractals") && j["dailyFractals"].is_array()) {
                    for (const auto& ej : j["dailyFractals"])
                        g_DailyFractals.push_back(deserializeEntry(ej));
                }
                if (j.contains("dailyBounties") && j["dailyBounties"].is_array()) {
                    for (const auto& ej : j["dailyBounties"])
                        g_DailyBounties.push_back(deserializeEntry(ej));
                }
            }

            // Load weekly data only if no weekly reset has occurred since fetch
            if (!weeklyStale) {
                if (j.contains("weeklyWings") && j["weeklyWings"].is_array()) {
                    for (const auto& ej : j["weeklyWings"])
                        g_WeeklyWings.push_back(deserializeEntry(ej));
                }
                if (j.contains("weeklyStrikes")) {
                    g_WeeklyStrikes = deserializeEntry(j["weeklyStrikes"]);
                }
            }

            // If we loaded any data, mark as fetched
            if (!g_DailyFractals.empty() || !g_WeeklyWings.empty()) {
                g_ClearsFetched = true;
                g_LastClearsCompletionQuery = std::chrono::steady_clock::now();
            }

            g_LastDailyReset = dailyReset;
            g_LastWeeklyReset = weeklyReset;
        }

        // If any data is stale, trigger a full re-fetch
        if (dailyStale || weeklyStale) {
            FetchClears();
        }
    } catch (...) {}
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: hSelf = hModule; break;
    case DLL_PROCESS_DETACH: break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    }
    return TRUE;
}

// Forward declarations
void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void ProcessKeybind(const char* aIdentifier, bool aIsRelease);
void AddonRender();
void AddonOptions();
void OnMumbleIdentityUpdated(void* eventArgs);

// Icon size for item icons
static const float ICON_SIZE = 40.0f;
static const float SMALL_ICON_SIZE = 24.0f;
static const float TRAIT_ICON_SIZE = 32.0f;
static const float MINOR_TRAIT_SIZE = 26.0f;
static const float SPEC_PORTRAIT_SIZE = 100.0f;

// Equipment slot display order (paper doll layout)
static const char* EQUIPMENT_SLOTS_LEFT[] = {
    "Helm", "Shoulders", "Coat", "Gloves", "Leggings", "Boots"
};
static const char* EQUIPMENT_SLOTS_RIGHT[] = {
    "Backpack", "Accessory1", "Accessory2", "Amulet", "Ring1", "Ring2"
};
static const char* EQUIPMENT_SLOTS_WEAPONS[] = {
    "WeaponA1", "WeaponA2", "WeaponB1", "WeaponB2",
    "WeaponAquaticA", "WeaponAquaticB"
};

static const char* SlotDisplayName(const std::string& slot) {
    if (slot == "Helm") return "Helm";
    if (slot == "Shoulders") return "Shoulders";
    if (slot == "Coat") return "Chest";
    if (slot == "Gloves") return "Gloves";
    if (slot == "Leggings") return "Leggings";
    if (slot == "Boots") return "Boots";
    if (slot == "Backpack") return "Back";
    if (slot == "Accessory1") return "Accessory 1";
    if (slot == "Accessory2") return "Accessory 2";
    if (slot == "Amulet") return "Amulet";
    if (slot == "Ring1") return "Ring 1";
    if (slot == "Ring2") return "Ring 2";
    if (slot == "WeaponA1") return "Weapon A1";
    if (slot == "WeaponA2") return "Weapon A2";
    if (slot == "WeaponB1") return "Weapon B1";
    if (slot == "WeaponB2") return "Weapon B2";
    if (slot == "WeaponAquaticA") return "Aquatic A";
    if (slot == "WeaponAquaticB") return "Aquatic B";
    if (slot == "HelmAquatic") return "Aqua Helm";
    if (slot == "Sickle") return "Sickle";
    if (slot == "Axe") return "Logging Axe";
    if (slot == "Pick") return "Mining Pick";
    if (slot == "FishingRod") return "Fishing Rod";
    if (slot == "FishingBait") return "Bait";
    if (slot == "FishingLure") return "Lure";
    if (slot == "PowerCore") return "Jade Bot Core";
    if (slot == "Relic") return "Relic";
    if (slot == "SensoryArray") return "Sensory Array";
    if (slot == "ServiceChip") return "Service Chip";
    return slot.c_str();
}

// Fetch item/skin detail data for selected character on-demand
static void FetchDetailsForCharacter(const AlterEgo::Character& ch) {
    std::vector<uint32_t> item_ids;
    std::vector<uint32_t> skin_ids;
    std::vector<uint32_t> spec_ids;
    std::vector<uint32_t> stat_ids;

    for (const auto& eq : ch.equipment) {
        if (eq.id != 0) item_ids.push_back(eq.id);
        if (eq.skin != 0) skin_ids.push_back(eq.skin);
        if (eq.stat_id != 0) stat_ids.push_back(eq.stat_id);
        for (auto u : eq.upgrades) if (u != 0) item_ids.push_back(u);
        for (auto inf : eq.infusions) if (inf != 0) item_ids.push_back(inf);
    }

    std::vector<uint32_t> skill_ids;

    for (const auto& bt : ch.build_tabs) {
        for (int i = 0; i < 3; i++) {
            if (bt.specializations[i].spec_id != 0)
                spec_ids.push_back(bt.specializations[i].spec_id);
        }
        // Collect skill IDs
        auto addSkill = [&](uint32_t id) { if (id != 0) skill_ids.push_back(id); };
        addSkill(bt.terrestrial_skills.heal);
        for (int i = 0; i < 3; i++) addSkill(bt.terrestrial_skills.utilities[i]);
        addSkill(bt.terrestrial_skills.elite);
        addSkill(bt.aquatic_skills.heal);
        for (int i = 0; i < 3; i++) addSkill(bt.aquatic_skills.utilities[i]);
        addSkill(bt.aquatic_skills.elite);
    }

    if (!item_ids.empty()) AlterEgo::GW2API::FetchItemDetailsAsync(item_ids);
    if (!skin_ids.empty()) AlterEgo::GW2API::FetchSkinDetailsAsync(skin_ids);
    if (!spec_ids.empty()) AlterEgo::GW2API::FetchSpecDetailsAsync(spec_ids);
    if (!skill_ids.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(skill_ids);
    if (!stat_ids.empty()) AlterEgo::GW2API::FetchItemStatDetailsAsync(stat_ids);

    // Query skin unlock status from H&S cache (instant, no API call)
    if (!skin_ids.empty()) AlterEgo::GW2API::QuerySkinUnlocks(skin_ids);

    // Fetch profession icon for equipment panel silhouette
    if (!ch.profession.empty()) AlterEgo::GW2API::FetchProfessionInfoAsync(ch.profession);
}

// Render a single equipment slot with icon and tooltip
static void RenderEquipmentSlot(const AlterEgo::EquipmentItem* eq, const char* slotName) {
    ImGui::PushID(slotName);

    if (!eq || eq->id == 0) {
        // Empty slot
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
            IM_COL32(40, 40, 40, 180));
        ImGui::GetWindowDrawList()->AddRect(
            pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
            IM_COL32(80, 80, 80, 200));
        ImGui::Dummy(ImVec2(ICON_SIZE, ICON_SIZE));
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s (Empty)", slotName);
            ImGui::EndTooltip();
        }
    } else {
        // Try to show skin icon first, fall back to item icon
        uint32_t display_id = eq->skin ? eq->skin : eq->id;
        bool is_skin = eq->skin != 0;

        const AlterEgo::ItemInfo* item_info = AlterEgo::GW2API::GetItemInfo(eq->id);
        const AlterEgo::SkinInfo* skin_info = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;

        std::string icon_url;
        std::string display_name = slotName;
        std::string rarity = "Basic";

        if (skin_info) {
            icon_url = skin_info->icon_url;
            display_name = skin_info->name;
            rarity = skin_info->rarity;
        } else if (item_info) {
            icon_url = item_info->icon_url;
            display_name = item_info->name;
            rarity = item_info->rarity;
        }

        // Use item rarity for border color (actual item, not skin)
        if (item_info) rarity = item_info->rarity;

        Texture_t* tex = AlterEgo::IconManager::GetIcon(display_id);
        if (tex && tex->Resource) {
            ImVec4 borderColor = GetRarityColor(rarity);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRect(
                ImVec2(pos.x - 1, pos.y - 1),
                ImVec2(pos.x + ICON_SIZE + 1, pos.y + ICON_SIZE + 1),
                ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, 2.0f);
            ImGui::Image(tex->Resource, ImVec2(ICON_SIZE, ICON_SIZE));
        } else {
            ImVec4 col = GetRarityColor(rarity);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
                ImGui::ColorConvertFloat4ToU32(ImVec4(col.x * 0.3f, col.y * 0.3f, col.z * 0.3f, 0.5f)));
            ImGui::GetWindowDrawList()->AddRect(
                pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
                ImGui::ColorConvertFloat4ToU32(col), 0.0f, 0, 1.0f);
            ImGui::Dummy(ImVec2(ICON_SIZE, ICON_SIZE));

            if (!icon_url.empty())
                AlterEgo::IconManager::RequestIcon(display_id, icon_url);
        }

        // Tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();

            if (tex && tex->Resource) {
                ImGui::Image(tex->Resource, ImVec2(48, 48));
                ImGui::SameLine();
            }
            ImGui::BeginGroup();
            ImGui::TextColored(GetRarityColor(rarity), "%s", display_name.c_str());

            // Defense or weapon strength from item details
            if (item_info && item_info->details.is_object()) {
                const auto& det = item_info->details;
                if (det.contains("defense") && det["defense"].get<int>() > 0) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Defense: %d",
                        det["defense"].get<int>());
                }
                if (det.contains("min_power") && det.contains("max_power")) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Weapon Strength: %d - %d",
                        det["min_power"].get<int>(), det["max_power"].get<int>());
                }
            }

            // Stat type name (e.g. "Berserker's", "Minstrel's")
            if (eq->stat_id != 0) {
                const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(eq->stat_id);
                if (statInfo && !statInfo->name.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), "%s", statInfo->name.c_str());
                }
            } else if (item_info && item_info->details.is_object() &&
                       item_info->details.contains("infix_upgrade") &&
                       item_info->details["infix_upgrade"].contains("id")) {
                uint32_t fixedStatId = item_info->details["infix_upgrade"]["id"].get<uint32_t>();
                const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(fixedStatId);
                if (statInfo && !statInfo->name.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), "%s", statInfo->name.c_str());
                }
            }

            // Attribute bonuses
            if (!eq->attributes.empty()) {
                for (const auto& [attr, val] : eq->attributes) {
                    // Format attribute name for display (CamelCase → spaced)
                    std::string displayAttr = attr;
                    if (attr == "BoonDuration") displayAttr = "Concentration";
                    else if (attr == "ConditionDuration") displayAttr = "Expertise";
                    else if (attr == "ConditionDamage") displayAttr = "Condition Damage";
                    else if (attr == "CritDamage") displayAttr = "Ferocity";
                    else if (attr == "HealingPower") displayAttr = "Healing Power";
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "+%d %s", val, displayAttr.c_str());
                }
            } else if (item_info && item_info->details.is_object() &&
                       item_info->details.contains("infix_upgrade") &&
                       item_info->details["infix_upgrade"].contains("attributes")) {
                // Fallback: use fixed stats from item details
                for (const auto& attr : item_info->details["infix_upgrade"]["attributes"]) {
                    std::string name = attr.value("attribute", "");
                    int mod = attr.value("modifier", 0);
                    if (mod > 0 && !name.empty()) {
                        std::string displayAttr = name;
                        if (name == "BoonDuration") displayAttr = "Concentration";
                        else if (name == "ConditionDuration") displayAttr = "Expertise";
                        else if (name == "ConditionDamage") displayAttr = "Condition Damage";
                        else if (name == "CritDamage") displayAttr = "Ferocity";
                        else if (name == "HealingPower") displayAttr = "Healing Power";
                        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "+%d %s", mod, displayAttr.c_str());
                    }
                }
            }

            // Infusions
            for (auto inf_id : eq->infusions) {
                if (inf_id == 0) continue;
                const auto* iinfo = AlterEgo::GW2API::GetItemInfo(inf_id);
                if (iinfo) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "%s", iinfo->name.c_str());
                    // Show infusion stats from its details
                    if (iinfo->details.is_object() && iinfo->details.contains("infix_upgrade") &&
                        iinfo->details["infix_upgrade"].contains("attributes")) {
                        for (const auto& attr : iinfo->details["infix_upgrade"]["attributes"]) {
                            std::string name = attr.value("attribute", "");
                            int mod = attr.value("modifier", 0);
                            if (mod > 0) {
                                ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "+%d %s", mod, name.c_str());
                            }
                        }
                    }
                    if (!iinfo->description.empty()) {
                        ImGui::PushTextWrapPos(300.0f);
                        std::string desc = StripGW2Markup(iinfo->description);
                        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "%s", desc.c_str());
                        ImGui::PopTextWrapPos();
                    }
                }
            }

            // Upgrades (Runes/Sigils)
            for (auto upgrade_id : eq->upgrades) {
                if (upgrade_id == 0) continue;
                const auto* uinfo = AlterEgo::GW2API::GetItemInfo(upgrade_id);
                if (uinfo) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.9f, 1.0f), "%s", uinfo->name.c_str());
                    // Show rune bonuses
                    if (uinfo->details.is_object() && uinfo->details.contains("bonuses") &&
                        uinfo->details["bonuses"].is_array()) {
                        int idx = 1;
                        for (const auto& bonus : uinfo->details["bonuses"]) {
                            ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.9f, 1.0f), "(%d): %s",
                                idx++, bonus.get<std::string>().c_str());
                        }
                    }
                    // Show sigil description
                    if (uinfo->details.is_object() && uinfo->details.contains("infix_upgrade") &&
                        uinfo->details["infix_upgrade"].contains("buff") &&
                        uinfo->details["infix_upgrade"]["buff"].contains("description")) {
                        ImGui::PushTextWrapPos(300.0f);
                        std::string udesc = StripGW2Markup(uinfo->details["infix_upgrade"]["buff"]["description"].get<std::string>());
                        ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.9f, 1.0f), "%s", udesc.c_str());
                        ImGui::PopTextWrapPos();
                    }
                }
            }

            // Transmuted skin
            if (skin_info && item_info && skin_info->name != item_info->name) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Transmuted");
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", skin_info->name.c_str());
            }

            // Rarity + type
            if (item_info) {
                ImGui::Spacing();
                ImGui::TextColored(GetRarityColor(rarity), "%s", item_info->rarity.c_str());
                if (item_info->details.is_object()) {
                    std::string subtype;
                    if (item_info->details.contains("weight_class"))
                        subtype = item_info->details["weight_class"].get<std::string>() + " ";
                    if (item_info->details.contains("type"))
                        subtype += WeaponDisplayName(item_info->details["type"].get<std::string>());
                    else
                        subtype += WeaponDisplayName(item_info->type);
                    if (!subtype.empty())
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", subtype.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", WeaponDisplayName(item_info->type));
                }
            }

            // Description
            if (item_info && !item_info->description.empty()) {
                ImGui::PushTextWrapPos(300.0f);
                std::string idesc = StripGW2Markup(item_info->description);
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 1.0f), "%s", idesc.c_str());
                ImGui::PopTextWrapPos();
            }

            // Skin unlock status from H&S cache
            if (eq->skin != 0 && AlterEgo::GW2API::HasSkinUnlockData(eq->skin)) {
                if (AlterEgo::GW2API::IsSkinUnlocked(eq->skin)) {
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Skin Unlocked");
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Skin Locked");
                }
            }

            // Dyes (limit to skin's actual dye channel count)
            if (!eq->dyes.empty()) {
                int maxDyes = (int)eq->dyes.size();
                if (skin_info && skin_info->dye_slot_count >= 0) {
                    maxDyes = std::min(maxDyes, skin_info->dye_slot_count);
                }
                bool hasAny = false;
                for (int di = 0; di < maxDyes; di++) {
                    if (eq->dyes[di] == 0) continue;
                    const auto* dye = AlterEgo::GW2API::GetDyeColor(eq->dyes[di]);
                    if (dye) {
                        if (!hasAny) { ImGui::Spacing(); hasAny = true; }
                        ImVec4 dyeCol((float)dye->r / 255.0f, (float)dye->g / 255.0f,
                                      (float)dye->b / 255.0f, 1.0f);
                        ImGui::ColorButton("##dye", dyeCol, 0, ImVec2(12, 12));
                        ImGui::SameLine();
                        ImGui::Text("%s", dye->name.c_str());
                    }
                }
            }

            ImGui::EndGroup();
            ImGui::EndTooltip();
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextItem("##eq_ctx")) {
            if (item_info) {
                if (ImGui::MenuItem("Copy Item Chat Link")) {
                    std::string link = AlterEgo::ChatLink::EncodeItem(eq->id, 1, eq->skin,
                        eq->upgrades.size() > 0 ? eq->upgrades[0] : 0,
                        eq->upgrades.size() > 1 ? eq->upgrades[1] : 0);
                    CopyToClipboard(link);
                }
                if (!item_info->chat_link.empty()) {
                    if (ImGui::MenuItem("Copy API Chat Link")) {
                        CopyToClipboard(item_info->chat_link);
                    }
                }
            }
            if (skin_info) {
                if (ImGui::MenuItem("Copy Skin Chat Link")) {
                    std::string link = AlterEgo::ChatLink::EncodeSkin(eq->skin);
                    CopyToClipboard(link);
                }
            }
            ImGui::Separator();
            // Find item in H&S (opens H&S search window)
            if (item_info && !item_info->name.empty()) {
                if (ImGui::MenuItem("Find in Hoard & Seek")) {
                    APIDefs->Events_Raise(EV_HOARD_SEARCH, (void*)item_info->name.c_str());
                }
            }
            // Show item locations from H&S cache
            if (item_info) {
                const auto* loc = AlterEgo::GW2API::GetItemLocation(eq->id);
                if (loc) {
                    if (ImGui::BeginMenu("Item Locations")) {
                        ImGui::Text("Total: %d", loc->total_count);
                        ImGui::Separator();
                        for (const auto& entry : loc->locations) {
                            ImGui::Text("%s", entry.location.c_str());
                            ImGui::SameLine(150);
                            if (!entry.sublocation.empty()) {
                                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", entry.sublocation.c_str());
                                ImGui::SameLine(280);
                            }
                            ImGui::Text("x%d", entry.count);
                        }
                        ImGui::EndMenu();
                    }
                } else {
                    if (ImGui::MenuItem("Query Item Locations")) {
                        AlterEgo::GW2API::QueryItemLocation(eq->id);
                    }
                }
            }
            ImGui::EndPopup();
        }
    }

    ImGui::PopID();
}

// Find equipment for a slot + tab
static const AlterEgo::EquipmentItem* FindEquipment(
    const AlterEgo::Character& ch, const std::string& slot, int tab)
{
    for (const auto& eq : ch.equipment) {
        if (eq.slot == slot && (eq.tab == tab || eq.tab == 0)) {
            return &eq;
        }
    }
    return nullptr;
}

// Helper: render one equipment row with label LEFT of icon (for right column)
static void RenderEquipRowReverse(const AlterEgo::Character& ch, const char* slot, int tab) {
    const auto* eq = FindEquipment(ch, slot, tab);

    // Determine display name and rarity
    std::string name = "---";
    std::string rarity = "Basic";
    if (eq && eq->id != 0) {
        const auto* info = AlterEgo::GW2API::GetItemInfo(eq->id);
        const auto* skin = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;
        if (info) rarity = info->rarity;
        if (std::string(slot) == "Relic" && info && info->rarity == "Legendary") {
            name = "Legendary Relic";
        } else if (skin) {
            name = skin->name;
        } else if (info) {
            name = info->name;
        } else {
            name = SlotDisplayName(slot);
        }
    }

    // Right-align: push label to the right so icon ends at the right edge
    float colW = ImGui::GetContentRegionAvail().x;
    float textW = ImGui::CalcTextSize(name.c_str()).x;
    float totalW = textW + 4.0f + ICON_SIZE; // text + gap + icon
    float offset = colW - totalW;
    if (offset > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    ImGui::AlignTextToFramePadding();
    if (eq && eq->id != 0) {
        ImGui::TextColored(GetRarityColor(rarity), "%s", name.c_str());
    } else {
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "%s", name.c_str());
    }
    ImGui::SameLine();
    RenderEquipmentSlot(eq, SlotDisplayName(slot));
}

// Helper: render one equipment row (icon + name) for the paper-doll grid
static void RenderEquipRow(const AlterEgo::Character& ch, const char* slot, int tab) {
    const auto* eq = FindEquipment(ch, slot, tab);
    RenderEquipmentSlot(eq, SlotDisplayName(slot));
    ImGui::SameLine();
    if (eq && eq->id != 0) {
        const auto* info = AlterEgo::GW2API::GetItemInfo(eq->id);
        const auto* skin = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;
        std::string rarity = info ? info->rarity : "Basic";

        // Determine display name
        std::string name;
        if (std::string(slot) == "Relic" && info && info->rarity == "Legendary") {
            name = "Legendary Relic";
        } else if (skin) {
            name = skin->name;
        } else if (info) {
            name = info->name;
        } else {
            name = SlotDisplayName(slot);
        }

        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(GetRarityColor(rarity), "%s", name.c_str());
    } else {
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "---");
    }
}

// Draw a race silhouette as a semi-transparent background overlay
static void DrawRaceSilhouette(ImDrawList* dl, const std::string& race, const std::string& gender,
                                ImVec2 center, float height) {
    ImU32 col = IM_COL32(255, 255, 255, 25); // Very faint white
    float s = height; // total height scale
    bool female = (gender == "Female");

    // Helper: offset from center
    auto P = [&](float x, float y) -> ImVec2 {
        return ImVec2(center.x + x * s, center.y + y * s);
    };

    if (race == "Asura") {
        // Large head, big ears, tiny body
        float headR = 0.18f;
        dl->AddCircleFilled(P(0, -0.30f), headR * s, col, 24);
        // Ears (triangles)
        dl->AddTriangleFilled(P(-0.15f, -0.35f), P(-0.28f, -0.48f), P(-0.10f, -0.44f), col);
        dl->AddTriangleFilled(P(0.15f, -0.35f), P(0.28f, -0.48f), P(0.10f, -0.44f), col);
        // Eyes (dark spots)
        ImU32 eyeCol = IM_COL32(0, 0, 0, 40);
        dl->AddCircleFilled(P(-0.07f, -0.32f), 0.035f * s, eyeCol, 12);
        dl->AddCircleFilled(P(0.07f, -0.32f), 0.035f * s, eyeCol, 12);
        // Body (small)
        float bw = female ? 0.09f : 0.11f;
        ImVec2 body[] = { P(-bw, -0.12f), P(bw, -0.12f), P(bw * 0.8f, 0.15f), P(-bw * 0.8f, 0.15f) };
        dl->AddConvexPolyFilled(body, 4, col);
        // Arms
        ImVec2 armL[] = { P(-bw, -0.10f), P(-bw - 0.06f, -0.08f), P(-bw - 0.08f, 0.10f), P(-bw, 0.08f) };
        ImVec2 armR[] = { P(bw, -0.10f), P(bw + 0.06f, -0.08f), P(bw + 0.08f, 0.10f), P(bw, 0.08f) };
        dl->AddConvexPolyFilled(armL, 4, col);
        dl->AddConvexPolyFilled(armR, 4, col);
        // Legs
        ImVec2 legL[] = { P(-bw * 0.7f, 0.15f), P(-0.02f, 0.15f), P(-0.03f, 0.38f), P(-bw * 0.8f, 0.38f) };
        ImVec2 legR[] = { P(0.02f, 0.15f), P(bw * 0.7f, 0.15f), P(bw * 0.8f, 0.38f), P(0.03f, 0.38f) };
        dl->AddConvexPolyFilled(legL, 4, col);
        dl->AddConvexPolyFilled(legR, 4, col);
        // Feet (oversized)
        dl->AddCircleFilled(P(-0.06f, 0.40f), 0.04f * s, col, 12);
        dl->AddCircleFilled(P(0.06f, 0.40f), 0.04f * s, col, 12);
    }
    else if (race == "Charr") {
        // Hunched, horns, snout, tail, digitigrade legs
        float headR = 0.10f;
        ImVec2 headC = P(0.02f, -0.32f);
        dl->AddCircleFilled(headC, headR * s, col, 20);
        // Horns
        dl->AddTriangleFilled(P(-0.06f, -0.40f), P(-0.14f, -0.50f), P(-0.02f, -0.42f), col);
        dl->AddTriangleFilled(P(0.10f, -0.40f), P(0.18f, -0.50f), P(0.06f, -0.42f), col);
        // Snout
        dl->AddTriangleFilled(P(-0.02f, -0.28f), P(0.06f, -0.28f), P(0.02f, -0.22f), col);
        // Hunched torso
        float sw = female ? 0.14f : 0.17f;
        ImVec2 torso[] = { P(-sw, -0.22f), P(sw, -0.22f), P(sw * 0.7f, 0.12f), P(-sw * 0.7f, 0.12f) };
        dl->AddConvexPolyFilled(torso, 4, col);
        // Arms (thick)
        ImVec2 armL[] = { P(-sw, -0.20f), P(-sw - 0.07f, -0.16f), P(-sw - 0.05f, 0.08f), P(-sw + 0.02f, 0.06f) };
        ImVec2 armR[] = { P(sw, -0.20f), P(sw + 0.07f, -0.16f), P(sw + 0.05f, 0.08f), P(sw - 0.02f, 0.06f) };
        dl->AddConvexPolyFilled(armL, 4, col);
        dl->AddConvexPolyFilled(armR, 4, col);
        // Digitigrade legs
        ImVec2 legL[] = { P(-0.10f, 0.12f), P(-0.02f, 0.12f), P(-0.06f, 0.30f), P(-0.14f, 0.28f) };
        ImVec2 legR[] = { P(0.02f, 0.12f), P(0.10f, 0.12f), P(0.14f, 0.28f), P(0.06f, 0.30f) };
        dl->AddConvexPolyFilled(legL, 4, col);
        dl->AddConvexPolyFilled(legR, 4, col);
        // Lower legs (reversed knee)
        ImVec2 lowerL[] = { P(-0.14f, 0.28f), P(-0.06f, 0.30f), P(-0.10f, 0.46f), P(-0.16f, 0.44f) };
        ImVec2 lowerR[] = { P(0.06f, 0.30f), P(0.14f, 0.28f), P(0.16f, 0.44f), P(0.10f, 0.46f) };
        dl->AddConvexPolyFilled(lowerL, 4, col);
        dl->AddConvexPolyFilled(lowerR, 4, col);
        // Tail
        ImVec2 tail[] = { P(-0.08f, 0.08f), P(-0.06f, 0.12f), P(-0.24f, 0.20f), P(-0.26f, 0.16f) };
        dl->AddConvexPolyFilled(tail, 4, col);
    }
    else {
        // Human / Norn / Sylvari — humanoid with race-specific proportions
        float headR, shoulderW, hipW, legLen, neckY;
        if (race == "Norn") {
            headR = 0.09f;
            shoulderW = female ? 0.15f : 0.19f;
            hipW = female ? 0.13f : 0.14f;
            legLen = 0.22f;
            neckY = -0.33f;
        } else if (race == "Sylvari") {
            headR = 0.08f;
            shoulderW = female ? 0.11f : 0.13f;
            hipW = female ? 0.10f : 0.10f;
            legLen = 0.22f;
            neckY = -0.34f;
        } else { // Human
            headR = 0.085f;
            shoulderW = female ? 0.12f : 0.15f;
            hipW = female ? 0.11f : 0.11f;
            legLen = 0.21f;
            neckY = -0.33f;
        }

        // Head
        dl->AddCircleFilled(P(0, neckY - headR * 0.8f), headR * s, col, 24);

        // Sylvari leaf crest
        if (race == "Sylvari") {
            dl->AddTriangleFilled(P(-0.04f, neckY - headR * 2.0f),
                                   P(0.04f, neckY - headR * 2.0f),
                                   P(0, neckY - headR * 3.2f), col);
        }

        // Torso
        float torsoBot = 0.08f;
        ImVec2 torso[] = {
            P(-shoulderW, neckY + 0.02f), P(shoulderW, neckY + 0.02f),
            P(hipW, torsoBot), P(-hipW, torsoBot)
        };
        dl->AddConvexPolyFilled(torso, 4, col);

        // Arms
        float armW = 0.045f;
        ImVec2 armL[] = {
            P(-shoulderW, neckY + 0.03f), P(-shoulderW - armW, neckY + 0.05f),
            P(-shoulderW - armW + 0.01f, 0.14f), P(-shoulderW + 0.02f, 0.12f)
        };
        ImVec2 armR[] = {
            P(shoulderW, neckY + 0.03f), P(shoulderW + armW, neckY + 0.05f),
            P(shoulderW + armW - 0.01f, 0.14f), P(shoulderW - 0.02f, 0.12f)
        };
        dl->AddConvexPolyFilled(armL, 4, col);
        dl->AddConvexPolyFilled(armR, 4, col);

        // Legs
        float legW = hipW * 0.55f;
        ImVec2 legL[] = {
            P(-hipW + 0.01f, torsoBot), P(-0.02f, torsoBot),
            P(-0.03f, torsoBot + legLen), P(-hipW, torsoBot + legLen)
        };
        ImVec2 legR[] = {
            P(0.02f, torsoBot), P(hipW - 0.01f, torsoBot),
            P(hipW, torsoBot + legLen), P(0.03f, torsoBot + legLen)
        };
        dl->AddConvexPolyFilled(legL, 4, col);
        dl->AddConvexPolyFilled(legR, 4, col);

        // Feet
        dl->AddCircleFilled(P(-hipW * 0.5f, torsoBot + legLen + 0.01f), 0.03f * s, col, 10);
        dl->AddCircleFilled(P(hipW * 0.5f, torsoBot + legLen + 0.01f), 0.03f * s, col, 10);
    }
}

// Render the equipment panel (game-matching layout)
static void RenderEquipmentPanel(const AlterEgo::Character& ch) {
    // Add padding so icons don't touch the pane edges
    ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor.x + 6, cursor.y + 4));

    // Equipment tab selector
    int maxTab = 1;
    for (const auto& eq : ch.equipment) {
        if (eq.tab > maxTab) maxTab = eq.tab;
    }
    int tab = (g_SelectedEquipTab > 0) ? g_SelectedEquipTab : ch.active_equipment_tab;
    if (tab == 0) tab = 1;

    if (maxTab > 1) {
        ImGui::Text("Equipment Tab:");
        for (int t = 1; t <= maxTab; t++) {
            ImGui::SameLine();
            // Use tab name if available, otherwise just the number
            std::string label;
            auto nameIt = ch.equipment_tab_names.find(t);
            if (nameIt != ch.equipment_tab_names.end() && !nameIt->second.empty()) {
                label = nameIt->second;
            } else {
                label = std::to_string(t);
            }
            bool active = (tab == t);
            bool isActiveTab = (ch.active_equipment_tab == t);
            if (isActiveTab) label += " *";
            if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.7f, 1.0f));
            ImGui::PushID(t);
            if (ImGui::SmallButton(label.c_str())) {
                g_SelectedEquipTab = t;
                g_DetailsFetched = false;
            }
            ImGui::PopID();
            if (active) ImGui::PopStyleColor();
        }
    }

    ImGui::Spacing();

    // Record start position for silhouette overlay
    ImVec2 panelStart = ImGui::GetCursorScreenPos();

    // Main layout: Left (Armor + Weapons) | Right (Trinkets)
    if (ImGui::BeginTable("##equip_main", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("##left_col", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("##right_col", ImGuiTableColumnFlags_WidthStretch, 0.85f);

        ImGui::TableNextRow();

        // ===== LEFT COLUMN: Armor + Weapons =====
        ImGui::TableNextColumn();

        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Armor");
        ImGui::Separator();
        RenderEquipRow(ch, "Helm", tab);
        RenderEquipRow(ch, "Shoulders", tab);
        RenderEquipRow(ch, "Coat", tab);
        RenderEquipRow(ch, "Gloves", tab);
        RenderEquipRow(ch, "Leggings", tab);
        RenderEquipRow(ch, "Boots", tab);

        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Weapons");
        ImGui::Separator();
        if (ImGui::BeginTable("##wpn_sets", 2, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("##setA", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##setB", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); RenderEquipRow(ch, "WeaponA1", tab);
            ImGui::TableNextColumn(); RenderEquipRow(ch, "WeaponB1", tab);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); RenderEquipRow(ch, "WeaponA2", tab);
            ImGui::TableNextColumn(); RenderEquipRow(ch, "WeaponB2", tab);
            ImGui::EndTable();
        }

        // ===== RIGHT COLUMN: Trinkets =====
        ImGui::TableNextColumn();

        {
            float colW = ImGui::GetContentRegionAvail().x;
            float headerW = ImGui::CalcTextSize("Trinkets").x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colW - headerW);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Trinkets");
        }
        ImGui::Separator();
        RenderEquipRowReverse(ch, "Backpack", tab);
        RenderEquipRowReverse(ch, "Accessory1", tab);
        RenderEquipRowReverse(ch, "Accessory2", tab);
        RenderEquipRowReverse(ch, "Amulet", tab);
        RenderEquipRowReverse(ch, "Ring1", tab);
        RenderEquipRowReverse(ch, "Ring2", tab);
        RenderEquipRowReverse(ch, "Relic", tab);

        ImGui::EndTable();
    }

    // Draw race concept art centered in the equipment area as a semi-transparent overlay
    {
        ImVec2 panelEnd = ImGui::GetCursorScreenPos();
        float panelH = panelEnd.y - panelStart.y;
        float panelW = ImGui::GetContentRegionAvail().x;

        // Race concept art from GW2 wiki (CC BY-NC-SA 3.0)
        static const std::unordered_map<std::string, std::pair<uint32_t, std::string>> RACE_ART = {
            {"Asura",   {8000001, "https://wiki.guildwars2.com/images/3/3f/Asura_01_concept_art_%28white%29.jpg"}},
            {"Charr",   {8000002, "https://wiki.guildwars2.com/images/5/55/Charr_01_concept_art_%28white%29.jpg"}},
            {"Human",   {8000003, "https://wiki.guildwars2.com/images/e/e2/Human_01_concept_art_%28white%29.jpg"}},
            {"Norn",    {8000004, "https://wiki.guildwars2.com/images/a/a7/Norn_01_concept_art_%28white%29.jpg"}},
            {"Sylvari", {8000005, "https://wiki.guildwars2.com/images/5/54/Sylvari_01_concept_art_%28white%29.jpg"}}
        };

        auto artIt = RACE_ART.find(ch.race);
        if (artIt != RACE_ART.end()) {
            uint32_t artId = artIt->second.first;
            const std::string& artUrl = artIt->second.second;

            Texture_t* tex = AlterEgo::IconManager::GetIcon(artId);
            if (tex && tex->Resource) {
                // Scale art to fit panel height, maintain aspect ratio (these are portrait images ~3:4)
                float artAspect = 0.75f; // width/height approximate
                float artH = panelH * 0.90f;
                float artW = artH * artAspect;
                if (artW > panelW * 0.6f) {
                    artW = panelW * 0.6f;
                    artH = artW / artAspect;
                }
                if (artH > 40.0f) {
                    ImVec2 artPos(
                        panelStart.x + panelW * 0.5f - artW * 0.5f,
                        panelStart.y + panelH * 0.5f - artH * 0.5f
                    );
                    ImGui::GetWindowDrawList()->AddImage(
                        tex->Resource,
                        artPos,
                        ImVec2(artPos.x + artW, artPos.y + artH),
                        ImVec2(0, 0), ImVec2(1, 1),
                        IM_COL32(255, 255, 255, 35) // Faint overlay
                    );
                }
            } else {
                AlterEgo::IconManager::RequestIcon(artId, artUrl);
            }
        }
    }

    ImGui::Spacing();

    // ===== AQUATIC: Aquabreather + Aquatic Weapons =====
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Aquatic");
    ImGui::Separator();
    RenderEquipRow(ch, "HelmAquatic", tab);
    if (ImGui::BeginTable("##aq_wpn", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("##aqA", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##aqB", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); RenderEquipRow(ch, "WeaponAquaticA", tab);
        ImGui::TableNextColumn(); RenderEquipRow(ch, "WeaponAquaticB", tab);
        ImGui::EndTable();
    }
}

// Render the build panel
// Draw a marching ants animated line between two points
static void DrawDottedLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 color, float thickness = 1.5f, float dashLen = 4.0f, float gapLen = 3.0f) {
    float dx = b.x - a.x, dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float nx = dx / len, ny = dy / len;
    float cycle = dashLen + gapLen;
    float offset = fmodf((float)ImGui::GetTime() * 21.0f, cycle);
    float pos = offset - cycle;
    while (pos < len) {
        float t0 = pos > 0.0f ? pos : 0.0f;
        float t1 = pos + dashLen < len ? pos + dashLen : len;
        if (t1 > t0)
            dl->AddLine(ImVec2(a.x + nx * t0, a.y + ny * t0),
                        ImVec2(a.x + nx * t1, a.y + ny * t1), color, thickness);
        pos += cycle;
    }
}

// Helper: render a single trait icon with selected/dimmed state and tooltip
// Returns the center screen position of the rendered icon
static ImVec2 RenderTraitIcon(uint32_t trait_id, bool selected, bool isMinor, float size) {
    const auto* tinfo = AlterEgo::GW2API::GetTraitInfo(trait_id);

    ImGui::PushID((int)(trait_id + 2000000));

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);

    Texture_t* tex = AlterEgo::IconManager::GetIcon(trait_id + 2000000);
    if (tex && tex->Resource) {
        if (selected) {
            ImGui::GetWindowDrawList()->AddRect(
                ImVec2(pos.x - 1, pos.y - 1),
                ImVec2(pos.x + size + 1, pos.y + size + 1),
                IM_COL32(100, 220, 255, 255), 0.0f, 0, 2.0f);
        }

        if (!selected && !isMinor) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f);
            ImGui::Image(tex->Resource, ImVec2(size, size));
            ImGui::PopStyleVar();
        } else {
            ImGui::Image(tex->Resource, ImVec2(size, size));
        }
    } else {
        ImVec4 col = selected ? ImVec4(0.3f, 0.5f, 0.7f, 0.8f) : ImVec4(0.15f, 0.15f, 0.15f, 0.5f);
        ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size),
            ImGui::ColorConvertFloat4ToU32(col));
        ImGui::Dummy(ImVec2(size, size));

        if (tinfo && !tinfo->icon_url.empty())
            AlterEgo::IconManager::RequestIcon(trait_id + 2000000, tinfo->icon_url);
    }

    if (ImGui::IsItemHovered() && tinfo) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), "%s", tinfo->name.c_str());
        if (isMinor)
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Minor Trait");
        if (!tinfo->description.empty()) {
            ImGui::PushTextWrapPos(300.0f);
            std::string tdesc = StripGW2Markup(tinfo->description);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", tdesc.c_str());
            ImGui::PopTextWrapPos();
        }
        ImGui::EndTooltip();
    }

    ImGui::PopID();
    return center;
}

static void RenderSkillIcon(uint32_t skill_id, float size);
static bool DecodeBuildLink(const std::string& link, const std::string& name,
                            AlterEgo::GameMode mode, AlterEgo::SavedBuild& out);

static void RenderBuildPanel(const AlterEgo::Character& ch) {
    if (ch.build_tabs.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No build tabs available.");
        return;
    }

    // Build tab selector
    ImGui::Text("Build Tab:");
    for (int i = 0; i < (int)ch.build_tabs.size(); i++) {
        ImGui::SameLine();
        const auto& bt = ch.build_tabs[i];
        std::string label = bt.name.empty() ?
            ("Tab " + std::to_string(bt.tab)) : bt.name;
        if (bt.is_active) label += " *";

        bool active = (g_SelectedBuildTab == i);
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.7f, 1.0f));
        if (ImGui::SmallButton(label.c_str())) {
            g_SelectedBuildTab = i;
        }
        if (active) ImGui::PopStyleColor();
    }

    int buildIdx = g_SelectedBuildTab;
    if (buildIdx < 0 || buildIdx >= (int)ch.build_tabs.size()) {
        // Find active tab
        for (int i = 0; i < (int)ch.build_tabs.size(); i++) {
            if (ch.build_tabs[i].is_active) { buildIdx = i; break; }
        }
        if (buildIdx < 0) buildIdx = 0;
        g_SelectedBuildTab = buildIdx;
    }

    const auto& bt = ch.build_tabs[buildIdx];
    ImGui::Separator();

    // Record start position for profession/elite spec overlay
    ImVec2 buildPanelStart = ImGui::GetCursorScreenPos();

    // Specializations — game-like trait grid with dotted lines
    const float iconSz = TRAIT_ICON_SIZE; // uniform size for all trait icons
    const float colW = iconSz * 2.0f;

    for (int i = 0; i < 3; i++) {
        const auto& spec = bt.specializations[i];
        if (spec.spec_id == 0) {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "  (empty)");
            continue;
        }

        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(spec.spec_id);
        std::string specName = specInfo ? specInfo->name : ("Spec " + std::to_string(spec.spec_id));
        bool isElite = specInfo ? specInfo->elite : false;
        ImVec4 specColor = isElite ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

        ImGui::PushID(spec.spec_id);
        ImGui::TextColored(specColor, "%s", specName.c_str());

        if (specInfo && specInfo->major_traits.size() >= 9) {
            // Positions for dotted lines
            ImVec2 specCenter(0, 0);
            ImVec2 minorCenters[3] = {};
            ImVec2 selectedCenters[3] = {};
            bool hasSelected[3] = {false, false, false};

            // Outer 2-column table: [Spec Icon] [6-col trait grid]
            char outerId[32];
            snprintf(outerId, sizeof(outerId), "##so_%u", spec.spec_id);
            if (ImGui::BeginTable(outerId, 2,
                    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                    SPEC_PORTRAIT_SIZE + 8.0f);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                // Column 0: Spec icon, aligned with middle trait row
                ImGui::TableNextColumn();
                float gridH = iconSz * 3 + ImGui::GetStyle().CellPadding.y * 6;
                float padY = (gridH - SPEC_PORTRAIT_SIZE) * 0.5f;
                if (padY > 0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padY);

                if (specInfo && !specInfo->icon_url.empty()) {
                    Texture_t* tex = AlterEgo::IconManager::GetIcon(spec.spec_id);
                    if (tex && tex->Resource) {
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Image(tex->Resource,
                            ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    } else {
                        AlterEgo::IconManager::RequestIcon(spec.spec_id, specInfo->icon_url);
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Dummy(ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    }
                    // Spec tooltip
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::TextColored(specColor, "%s", specName.c_str());
                        const auto* specDesc = AlterEgo::GetSpecDescription(specName);
                        if (specDesc) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.9f, 1.0f),
                                "\"%s\"", specDesc->flavor);
                            ImGui::Spacing();
                            ImGui::PushTextWrapPos(350.0f);
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
                                "%s", specDesc->description);
                            ImGui::PopTextWrapPos();
                        } else {
                            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s%s",
                                specInfo->profession.c_str(),
                                isElite ? " (Elite)" : "");
                        }
                        ImGui::EndTooltip();
                    }
                }

                // Column 1: 6-column × 3-row trait grid
                ImGui::TableNextColumn();
                char innerId[32];
                snprintf(innerId, sizeof(innerId), "##tr_%u", spec.spec_id);
                if (ImGui::BeginTable(innerId, 6,
                        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                    for (int c = 0; c < 6; c++)
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, colW);

                    for (int row = 0; row < 3; row++) {
                        ImGui::TableNextRow();
                        for (int tier = 0; tier < 3; tier++) {
                            // Minor trait column: render only in middle row
                            ImGui::TableNextColumn();
                            if (row == 1 && tier < (int)specInfo->minor_traits.size()) {
                                minorCenters[tier] = RenderTraitIcon(
                                    specInfo->minor_traits[tier], true, true, iconSz);
                            }

                            // Major trait column
                            ImGui::TableNextColumn();
                            int majorIdx = tier * 3 + row;
                            uint32_t tid = specInfo->major_traits[majorIdx];
                            bool selected = ((uint32_t)spec.traits[tier] == tid);
                            ImVec2 c = RenderTraitIcon(tid, selected, false, iconSz);
                            if (selected) {
                                selectedCenters[tier] = c;
                                hasSelected[tier] = true;
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTable();
            }

            // Draw dotted lines connecting: spec → minor0 → sel0 → minor1 → sel1 → minor2 → sel2
            // Lines stop at icon edges (inset by radius) so they don't overlap icons
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 lineCol = IM_COL32(180, 230, 255, 180);
            float specR = SPEC_PORTRAIT_SIZE * 0.5f;
            float minorR = MINOR_TRAIT_SIZE * 0.5f;
            float majorR = iconSz * 0.5f;

            auto InsetLine = [](ImVec2 a, ImVec2 b, float rA, float rB, ImVec2& outA, ImVec2& outB) {
                float dx = b.x - a.x, dy = b.y - a.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len < rA + rB) return false;
                float nx = dx / len, ny = dy / len;
                outA = ImVec2(a.x + nx * rA, a.y + ny * rA);
                outB = ImVec2(b.x - nx * rB, b.y - ny * rB);
                return true;
            };

            ImVec2 prev = specCenter;
            float prevR = specR;
            for (int tier = 0; tier < 3; tier++) {
                if (minorCenters[tier].x > 0) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, minorCenters[tier], prevR, minorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = minorCenters[tier];
                    prevR = minorR;
                }
                if (hasSelected[tier]) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, selectedCenters[tier], prevR, majorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = selectedCenters[tier];
                    prevR = majorR;
                }
            }
        }

        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::Spacing();

    // Skill bar: Heal / Utility / Elite
    {
        // Fetch skill details if needed
        std::vector<uint32_t> toFetch;
        auto MaybeFetch = [&](uint32_t id) {
            if (id && !AlterEgo::GW2API::GetSkillInfo(id)) toFetch.push_back(id);
        };
        MaybeFetch(bt.terrestrial_skills.heal);
        for (int i = 0; i < 3; i++) MaybeFetch(bt.terrestrial_skills.utilities[i]);
        MaybeFetch(bt.terrestrial_skills.elite);
        if (!toFetch.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(toFetch);

        float skillSz = ICON_SIZE;
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Heal / Utility / Elite");
        ImGui::BeginGroup();
        RenderSkillIcon(bt.terrestrial_skills.heal, skillSz);
        ImGui::SameLine();
        for (int i = 0; i < 3; i++) {
            RenderSkillIcon(bt.terrestrial_skills.utilities[i], skillSz);
            ImGui::SameLine();
        }
        RenderSkillIcon(bt.terrestrial_skills.elite, skillSz);
        ImGui::EndGroup();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Copy build chat link button
    bool hasPalette = AlterEgo::GW2API::HasPaletteData(bt.profession);
    if (!hasPalette)
        AlterEgo::GW2API::FetchProfessionPaletteAsync(bt.profession);
    {
        if (ImGui::Button(hasPalette ? "Copy Build Chat Link" : "Copy Build Chat Link (loading...)") && hasPalette) {
            // Map profession name to code
            auto ProfCode = [](const std::string& p) -> uint8_t {
                if (p == "Guardian")     return 1;
                if (p == "Warrior")      return 2;
                if (p == "Engineer")     return 3;
                if (p == "Ranger")       return 4;
                if (p == "Thief")        return 5;
                if (p == "Elementalist") return 6;
                if (p == "Mesmer")       return 7;
                if (p == "Necromancer")  return 8;
                if (p == "Revenant")     return 9;
                return 0;
            };

            AlterEgo::DecodedBuildLink link{};
            link.profession = ProfCode(bt.profession);

            // Specializations: spec_id and trait choices are already in the right format
            for (int i = 0; i < 3; i++) {
                link.specs[i].spec_id = (uint8_t)bt.specializations[i].spec_id;
                for (int t = 0; t < 3; t++)
                    link.specs[i].traits[t] = (uint8_t)bt.specializations[i].traits[t];
            }

            // Skills: convert skill IDs to palette IDs
            auto ToPalette = [&](uint32_t skill_id) -> uint16_t {
                if (skill_id == 0) return 0;
                return AlterEgo::GW2API::GetPaletteIdFromSkill(bt.profession, skill_id);
            };
            link.terrestrial_skills[0] = ToPalette(bt.terrestrial_skills.heal);
            link.terrestrial_skills[1] = ToPalette(bt.terrestrial_skills.utilities[0]);
            link.terrestrial_skills[2] = ToPalette(bt.terrestrial_skills.utilities[1]);
            link.terrestrial_skills[3] = ToPalette(bt.terrestrial_skills.utilities[2]);
            link.terrestrial_skills[4] = ToPalette(bt.terrestrial_skills.elite);
            link.aquatic_skills[0] = ToPalette(bt.aquatic_skills.heal);
            link.aquatic_skills[1] = ToPalette(bt.aquatic_skills.utilities[0]);
            link.aquatic_skills[2] = ToPalette(bt.aquatic_skills.utilities[1]);
            link.aquatic_skills[3] = ToPalette(bt.aquatic_skills.utilities[2]);
            link.aquatic_skills[4] = ToPalette(bt.aquatic_skills.elite);

            // Ranger pets
            if (bt.profession == "Ranger") {
                link.pets[0] = (uint8_t)bt.pets.terrestrial[0];
                link.pets[1] = (uint8_t)bt.pets.terrestrial[1];
                link.pets[2] = (uint8_t)bt.pets.aquatic[0];
                link.pets[3] = (uint8_t)bt.pets.aquatic[1];
            }

            // Revenant legends: parse number from "Legend1" etc.
            if (bt.profession == "Revenant") {
                auto LegendByte = [](const std::string& s) -> uint8_t {
                    if (s.size() > 6 && s.substr(0, 6) == "Legend")
                        return (uint8_t)std::atoi(s.c_str() + 6);
                    return 0;
                };
                link.legends[0] = LegendByte(bt.legends.terrestrial[0]);
                link.legends[1] = LegendByte(bt.legends.terrestrial[1]);
                link.legends[2] = LegendByte(bt.legends.aquatic[0]);
                link.legends[3] = LegendByte(bt.legends.aquatic[1]);
            }

            // Weapons
            for (auto w : bt.weapons)
                link.weapons.push_back((uint16_t)w);

            std::string chatLink = AlterEgo::ChatLink::EncodeBuild(link);
            CopyToClipboard(chatLink);
            if (APIDefs) APIDefs->GUI_SendAlert("Build link copied!");
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        if (!hasPalette)
            ImGui::Text("Loading profession data...");
        else
            ImGui::Text("Export this build as a GW2 chat link");
        ImGui::EndTooltip();
    }

    // Save to Library button
    ImGui::SameLine();
    if (ImGui::Button(hasPalette ? "Save to Library" : "Save to Library (loading...)") && hasPalette) {
        // Build the chat link first
        auto ProfCodeLib = [](const std::string& p) -> uint8_t {
            if (p == "Guardian")     return 1;
            if (p == "Warrior")      return 2;
            if (p == "Engineer")     return 3;
            if (p == "Ranger")       return 4;
            if (p == "Thief")        return 5;
            if (p == "Elementalist") return 6;
            if (p == "Mesmer")       return 7;
            if (p == "Necromancer")  return 8;
            if (p == "Revenant")     return 9;
            return 0;
        };

        AlterEgo::DecodedBuildLink saveLink{};
        saveLink.profession = ProfCodeLib(bt.profession);
        for (int i = 0; i < 3; i++) {
            saveLink.specs[i].spec_id = (uint8_t)bt.specializations[i].spec_id;
            for (int t = 0; t < 3; t++)
                saveLink.specs[i].traits[t] = (uint8_t)bt.specializations[i].traits[t];
        }
        auto ToPaletteLib = [&](uint32_t skill_id) -> uint16_t {
            if (skill_id == 0) return 0;
            return AlterEgo::GW2API::GetPaletteIdFromSkill(bt.profession, skill_id);
        };
        saveLink.terrestrial_skills[0] = ToPaletteLib(bt.terrestrial_skills.heal);
        saveLink.terrestrial_skills[1] = ToPaletteLib(bt.terrestrial_skills.utilities[0]);
        saveLink.terrestrial_skills[2] = ToPaletteLib(bt.terrestrial_skills.utilities[1]);
        saveLink.terrestrial_skills[3] = ToPaletteLib(bt.terrestrial_skills.utilities[2]);
        saveLink.terrestrial_skills[4] = ToPaletteLib(bt.terrestrial_skills.elite);
        saveLink.aquatic_skills[0] = ToPaletteLib(bt.aquatic_skills.heal);
        saveLink.aquatic_skills[1] = ToPaletteLib(bt.aquatic_skills.utilities[0]);
        saveLink.aquatic_skills[2] = ToPaletteLib(bt.aquatic_skills.utilities[1]);
        saveLink.aquatic_skills[3] = ToPaletteLib(bt.aquatic_skills.utilities[2]);
        saveLink.aquatic_skills[4] = ToPaletteLib(bt.aquatic_skills.elite);
        if (bt.profession == "Ranger") {
            saveLink.pets[0] = (uint8_t)bt.pets.terrestrial[0];
            saveLink.pets[1] = (uint8_t)bt.pets.terrestrial[1];
            saveLink.pets[2] = (uint8_t)bt.pets.aquatic[0];
            saveLink.pets[3] = (uint8_t)bt.pets.aquatic[1];
        }
        if (bt.profession == "Revenant") {
            auto LegByte = [](const std::string& s) -> uint8_t {
                if (s.size() > 6 && s.substr(0, 6) == "Legend")
                    return (uint8_t)std::atoi(s.c_str() + 6);
                return 0;
            };
            saveLink.legends[0] = LegByte(bt.legends.terrestrial[0]);
            saveLink.legends[1] = LegByte(bt.legends.terrestrial[1]);
            saveLink.legends[2] = LegByte(bt.legends.aquatic[0]);
            saveLink.legends[3] = LegByte(bt.legends.aquatic[1]);
        }
        for (auto w : bt.weapons)
            saveLink.weapons.push_back((uint16_t)w);

        std::string chatLink = AlterEgo::ChatLink::EncodeBuild(saveLink);

        // Build a name from character + tab name
        std::string buildName = ch.name + " - " +
            (bt.name.empty() ? ("Tab " + std::to_string(bt.tab)) : bt.name);

        // Decode the link to populate SavedBuild fields
        AlterEgo::SavedBuild sb;
        if (DecodeBuildLink(chatLink, buildName, AlterEgo::GameMode::PvE, sb)) {
            AlterEgo::GW2API::AddSavedBuild(std::move(sb));
            if (APIDefs) APIDefs->GUI_SendAlert("Build saved to library!");
        }
    }

    // Draw profession/elite spec icon overlay in the top-right of the build panel
    {
        ImVec2 buildPanelEnd = ImGui::GetCursorScreenPos();
        float panelW = ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x - buildPanelStart.x;
        float overlaySize = 64.0f;
        float margin = 8.0f;

        // Determine which icon to show: elite spec if present, else base profession
        uint32_t overlayIconId = 0;
        std::string overlayIconUrl;
        std::string overlayTooltip;

        // Check for elite spec in this build's specializations
        for (int si = 0; si < 3; si++) {
            uint32_t sid = bt.specializations[si].spec_id;
            if (sid == 0) continue;
            const auto* sInfo = AlterEgo::GW2API::GetSpecInfo(sid);
            if (sInfo && sInfo->elite) {
                // Use offset ID to avoid collision with hex portrait icon
                overlayIconId = 7000000 + sid;
                overlayIconUrl = sInfo->profession_icon_big_url;
                overlayTooltip = sInfo->name;
                break;
            }
        }

        // Fall back to profession emblem from a core spec
        if (overlayIconId == 0) {
            for (int si = 0; si < 3; si++) {
                uint32_t sid = bt.specializations[si].spec_id;
                if (sid == 0) continue;
                const auto* sInfo = AlterEgo::GW2API::GetSpecInfo(sid);
                if (sInfo && !sInfo->profession_icon_big_url.empty()) {
                    overlayIconId = 7000000 + sid;
                    overlayIconUrl = sInfo->profession_icon_big_url;
                    overlayTooltip = ch.profession;
                    break;
                }
            }
        }

        if (overlayIconId != 0) {
            Texture_t* tex = AlterEgo::IconManager::GetIcon(overlayIconId);
            if (tex && tex->Resource) {
                ImVec2 iconPos(
                    buildPanelStart.x + panelW - overlaySize - margin,
                    buildPanelStart.y + margin
                );
                ImGui::GetWindowDrawList()->AddImage(
                    tex->Resource,
                    iconPos,
                    ImVec2(iconPos.x + overlaySize, iconPos.y + overlaySize),
                    ImVec2(0, 0), ImVec2(1, 1),
                    IM_COL32(255, 255, 255, 50)
                );
            } else if (!overlayIconUrl.empty()) {
                AlterEgo::IconManager::RequestIcon(overlayIconId, overlayIconUrl);
            }
        }
    }
}

// --- Build Library helpers ---

// Profession code to name mapping
static const char* ProfessionFromCode(uint8_t code) {
    switch (code) {
        case 1: return "Guardian";
        case 2: return "Warrior";
        case 3: return "Engineer";
        case 4: return "Ranger";
        case 5: return "Thief";
        case 6: return "Elementalist";
        case 7: return "Mesmer";
        case 8: return "Necromancer";
        case 9: return "Revenant";
        default: return "Unknown";
    }
}

static const char* GameModeNames[] = { "All", "PvE", "WvW", "PvP", "Raid", "Fractal", "Other" };
static const char* GameModeImportNames[] = { "PvE", "WvW", "PvP", "Raid", "Fractal", "Other" };

static AlterEgo::GameMode GameModeFromIndex(int idx) {
    switch (idx) {
        case 0: return AlterEgo::GameMode::PvE;
        case 1: return AlterEgo::GameMode::WvW;
        case 2: return AlterEgo::GameMode::PvP;
        case 3: return AlterEgo::GameMode::Raid;
        case 4: return AlterEgo::GameMode::Fractal;
        default: return AlterEgo::GameMode::Other;
    }
}

static bool GameModeMatchesFilter(AlterEgo::GameMode mode, int filter) {
    if (filter == 0) return true; // All
    switch (filter) {
        case 1: return mode == AlterEgo::GameMode::PvE;
        case 2: return mode == AlterEgo::GameMode::WvW;
        case 3: return mode == AlterEgo::GameMode::PvP;
        case 4: return mode == AlterEgo::GameMode::Raid;
        case 5: return mode == AlterEgo::GameMode::Fractal;
        default: return true;
    }
}

static const char* GameModeLabel(AlterEgo::GameMode m) {
    switch (m) {
        case AlterEgo::GameMode::PvE:     return "PvE";
        case AlterEgo::GameMode::WvW:     return "WvW";
        case AlterEgo::GameMode::PvP:     return "PvP";
        case AlterEgo::GameMode::Raid:    return "Raid";
        case AlterEgo::GameMode::Fractal: return "Fractal";
        default:                          return "Other";
    }
}

// Decode a build chat link and create a SavedBuild
static bool DecodeBuildLink(const std::string& link, const std::string& name,
                            AlterEgo::GameMode mode, AlterEgo::SavedBuild& out) {
    AlterEgo::DecodedBuildLink decoded;
    if (!AlterEgo::ChatLink::DecodeBuild(link, decoded)) return false;

    std::string profession = ProfessionFromCode(decoded.profession);
    if (profession == "Unknown") return false;

    out.name = name;
    out.chat_link = link;
    out.profession = profession;
    out.game_mode = mode;

    // Copy specializations — trait choices stored as indices (1=top, 2=mid, 3=bot)
    // Need to resolve to actual trait IDs using spec info
    for (int i = 0; i < 3; i++) {
        out.specializations[i].spec_id = decoded.specs[i].spec_id;
        // Store trait choices temporarily as indices; resolve to trait IDs below
        out.specializations[i].traits[0] = 0;
        out.specializations[i].traits[1] = 0;
        out.specializations[i].traits[2] = 0;
    }

    // Resolve trait choices to trait IDs using spec cache
    for (int i = 0; i < 3; i++) {
        if (decoded.specs[i].spec_id == 0) continue;
        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(decoded.specs[i].spec_id);
        if (specInfo && specInfo->major_traits.size() >= 9) {
            for (int tier = 0; tier < 3; tier++) {
                uint8_t choice = decoded.specs[i].traits[tier]; // 1=top, 2=mid, 3=bot
                if (choice >= 1 && choice <= 3) {
                    int majorIdx = tier * 3 + (choice - 1);
                    out.specializations[i].traits[tier] = (int)specInfo->major_traits[majorIdx];
                }
            }
        } else {
            // Spec info not loaded yet — store choice indices negated as placeholder
            for (int tier = 0; tier < 3; tier++) {
                out.specializations[i].traits[tier] = -(int)decoded.specs[i].traits[tier];
            }
        }
    }

    // Resolve skill palette IDs to skill IDs
    auto resolveSkill = [&](uint16_t palette_id) -> uint32_t {
        if (palette_id == 0) return 0;
        return AlterEgo::GW2API::GetSkillIdFromPalette(profession, palette_id);
    };

    out.terrestrial_skills.heal = resolveSkill(decoded.terrestrial_skills[0]);
    out.terrestrial_skills.utilities[0] = resolveSkill(decoded.terrestrial_skills[1]);
    out.terrestrial_skills.utilities[1] = resolveSkill(decoded.terrestrial_skills[2]);
    out.terrestrial_skills.utilities[2] = resolveSkill(decoded.terrestrial_skills[3]);
    out.terrestrial_skills.elite = resolveSkill(decoded.terrestrial_skills[4]);

    out.aquatic_skills.heal = resolveSkill(decoded.aquatic_skills[0]);
    out.aquatic_skills.utilities[0] = resolveSkill(decoded.aquatic_skills[1]);
    out.aquatic_skills.utilities[1] = resolveSkill(decoded.aquatic_skills[2]);
    out.aquatic_skills.utilities[2] = resolveSkill(decoded.aquatic_skills[3]);
    out.aquatic_skills.elite = resolveSkill(decoded.aquatic_skills[4]);

    // Pets (Ranger)
    out.pets.terrestrial[0] = decoded.pets[0];
    out.pets.terrestrial[1] = decoded.pets[1];
    out.pets.aquatic[0] = decoded.pets[2];
    out.pets.aquatic[1] = decoded.pets[3];

    // Legends (Revenant)
    for (int i = 0; i < 4; i++)
        out.legend_codes[i] = decoded.legends[i];

    // Weapons
    out.weapons.clear();
    for (auto w : decoded.weapons)
        out.weapons.push_back((uint32_t)w);

    return true;
}

// Fetch spec/trait/skill details needed for a saved build's preview
static void FetchDetailsForSavedBuild(const AlterEgo::SavedBuild& build) {
    std::vector<uint32_t> spec_ids, skill_ids;

    for (int i = 0; i < 3; i++) {
        if (build.specializations[i].spec_id != 0)
            spec_ids.push_back(build.specializations[i].spec_id);
    }

    auto addSkill = [&](uint32_t id) { if (id != 0) skill_ids.push_back(id); };
    addSkill(build.terrestrial_skills.heal);
    for (int i = 0; i < 3; i++) addSkill(build.terrestrial_skills.utilities[i]);
    addSkill(build.terrestrial_skills.elite);
    addSkill(build.aquatic_skills.heal);
    for (int i = 0; i < 3; i++) addSkill(build.aquatic_skills.utilities[i]);
    addSkill(build.aquatic_skills.elite);

    if (!spec_ids.empty()) AlterEgo::GW2API::FetchSpecDetailsAsync(spec_ids);
    if (!skill_ids.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(skill_ids);
}

// Render a skill icon with tooltip
static void RenderSkillIcon(uint32_t skill_id, float size) {
    if (skill_id == 0) {
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetCursorScreenPos(),
            ImVec2(ImGui::GetCursorScreenPos().x + size, ImGui::GetCursorScreenPos().y + size),
            IM_COL32(30, 30, 30, 200));
        ImGui::Dummy(ImVec2(size, size));
        return;
    }

    ImGui::PushID((int)(skill_id + 5000000));
    const auto* sinfo = AlterEgo::GW2API::GetSkillInfo(skill_id);
    Texture_t* tex = AlterEgo::IconManager::GetIcon(skill_id + 5000000);

    if (tex && tex->Resource) {
        ImGui::Image(tex->Resource, ImVec2(size, size));
    } else {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(pos,
            ImVec2(pos.x + size, pos.y + size), IM_COL32(40, 40, 60, 200));
        ImGui::Dummy(ImVec2(size, size));
        if (sinfo && !sinfo->icon_url.empty())
            AlterEgo::IconManager::RequestIcon(skill_id + 5000000, sinfo->icon_url);
    }

    if (ImGui::IsItemHovered() && sinfo) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), "%s", sinfo->name.c_str());
        if (!sinfo->description.empty()) {
            ImGui::PushTextWrapPos(300.0f);
            std::string sdesc = StripGW2Markup(sinfo->description);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", sdesc.c_str());
            ImGui::PopTextWrapPos();
        }
        ImGui::EndTooltip();
    }
    ImGui::PopID();
}

// =========================================================================
// Gear Customization Helpers
// =========================================================================

// Friendly attribute name mapping
static const char* FriendlyAttrName(const std::string& attr) {
    if (attr == "CritDamage") return "Ferocity";
    if (attr == "ConditionDamage") return "Condition Damage";
    if (attr == "ConditionDuration") return "Expertise";
    if (attr == "BoonDuration") return "Concentration";
    if (attr == "Healing") return "Healing Power";
    return attr.c_str();
}

// Curated rune list (name + summary)
struct RuneEntry { const char* name; const char* bonus; };
static const RuneEntry RUNE_LIST[] = {
    {"Superior Rune of the Scholar", "+Power, +Ferocity, +10% dmg >90% HP"},
    {"Superior Rune of the Eagle", "+Power, +Precision, +Ferocity, +10% dmg >80% HP"},
    {"Superior Rune of Strength", "+Power, +Ferocity, +5% dmg on might"},
    {"Superior Rune of the Dragonhunter", "+Power, +Ferocity, +7% vs burning foes"},
    {"Superior Rune of the Berserker", "+Power, +Ferocity, +5% crit dmg"},
    {"Superior Rune of the Thief", "+Power, +Precision, +10% dmg from stealth"},
    {"Superior Rune of the Firebrand", "+Condi Dmg, +Expertise, +7% burning dmg"},
    {"Superior Rune of Balthazar", "+Condi Dmg, +Burning duration"},
    {"Superior Rune of the Trapper", "+Condi Dmg, +Expertise, +10% condi dur"},
    {"Superior Rune of the Afflicted", "+Condi Dmg, +Expertise"},
    {"Superior Rune of Tormenting", "+Condi Dmg, +Expertise, torment on hit"},
    {"Superior Rune of the Tempest", "+Ferocity, +Precision, stun on crit"},
    {"Superior Rune of Surging", "+Power, +Ferocity, swiftness on swap"},
    {"Superior Rune of the Pack", "+Power, +Precision, might/fury/swiftness"},
    {"Superior Rune of Vampirism", "+Power, +Ferocity, lifesteal"},
    {"Superior Rune of the Monk", "+Healing, +Outgoing Heal, +Boon Duration"},
    {"Superior Rune of the Water", "+Healing, +Boon Duration"},
    {"Superior Rune of Durability", "+Toughness, +Vitality, +Boon Duration"},
    {"Superior Rune of the Defender", "+Toughness, +Vitality"},
    {"Superior Rune of the Traveler", "+All stats, +25% movement"},
    {"Superior Rune of the Rebirth", "+Healing, +Boon Duration, auto-rez"},
    {"Superior Rune of the Nightmare", "+Condi Dmg, +Condi Duration"},
    {"Superior Rune of Thorns", "+Condi Dmg, +Expertise, +bleeding"},
    {"Superior Rune of Permeating Wrath", "+Power, +Precision, +burning AoE"},
    {"Superior Rune of the Citadel", "+Power, +Ferocity, +might on kill"},
    {"Superior Rune of Fireworks", "+Power, +Boon Duration"},
    {"Superior Rune of the Grove", "+Condi Dmg, +Healing"},
    {"Superior Rune of Sanctuary", "+Vitality, +Toughness, barrier on heal"},
    {"Superior Rune of Snowfall", "+Power, +Precision, superspeed"},
    {"Superior Rune of the Sunless", "+Condi Dmg, +Expertise, +torment"},
    {"Superior Rune of Resistance", "+Vitality, +Toughness, Resistance"},
    {"Superior Rune of the Weaver", "+Power, +Condi Dmg, +Ferocity"},
    {"Superior Rune of Scavenging", "+MF, +Power, +Precision"},
    {"Superior Rune of Antitoxin", "+Vitality, +Healing, condi cleanse"},
    {"Superior Rune of Aristocracy", "+Power, +Precision, +might dur"},
    {"Superior Rune of Rage", "+Power, +Ferocity, fury on crit"},
};
static const int RUNE_COUNT = sizeof(RUNE_LIST) / sizeof(RUNE_LIST[0]);

// Curated sigil list
struct SigilEntry { const char* name; const char* bonus; };
static const SigilEntry SIGIL_LIST[] = {
    {"Superior Sigil of Force", "+5% damage"},
    {"Superior Sigil of Impact", "+3% stun duration"},
    {"Superior Sigil of Accuracy", "+7% crit chance"},
    {"Superior Sigil of the Night", "+10% dmg at night"},
    {"Superior Sigil of Slaying (Undead)", "+10% vs undead"},
    {"Superior Sigil of Slaying (Demon)", "+10% vs demons"},
    {"Superior Sigil of Air", "Lightning strike on crit"},
    {"Superior Sigil of Fire", "AoE fire on crit"},
    {"Superior Sigil of Earth", "Bleed on crit"},
    {"Superior Sigil of Geomancy", "Bleed AoE on swap"},
    {"Superior Sigil of Doom", "Poison on swap"},
    {"Superior Sigil of Torment", "Torment on crit"},
    {"Superior Sigil of Malice", "+10% condition duration"},
    {"Superior Sigil of Smoldering", "+20% burning duration"},
    {"Superior Sigil of Venom", "+20% poison duration"},
    {"Superior Sigil of Agony", "+20% torment duration"},
    {"Superior Sigil of Bursting", "+5% condition damage"},
    {"Superior Sigil of Concentration", "+10% boon duration"},
    {"Superior Sigil of Transference", "+10% outgoing heal"},
    {"Superior Sigil of Renewal", "Heal on kill"},
    {"Superior Sigil of Blood", "Lifesteal on crit"},
    {"Superior Sigil of Strength", "Might on crit"},
    {"Superior Sigil of Severance", "Quickness on cc"},
    {"Superior Sigil of Energy", "Endurance on swap"},
    {"Superior Sigil of Paralyzation", "+30% stun duration"},
    {"Superior Sigil of Absorption", "Steal boon on hit"},
    {"Superior Sigil of Cleansing", "Condi cleanse on swap"},
    {"Superior Sigil of Draining", "Steal HP on swap"},
    {"Superior Sigil of Exploitation", "+3% dmg to <50% HP"},
    {"Superior Sigil of Demons", "+10% vs demons"},
    {"Superior Sigil of Serpent Slaying", "+10% vs destroyers"},
    {"Superior Sigil of Courage", "Might on swap"},
};
static const int SIGIL_COUNT = sizeof(SIGIL_LIST) / sizeof(SIGIL_LIST[0]);

// Curated relic list
struct RelicEntry { const char* name; const char* bonus; };
static const RelicEntry RELIC_LIST[] = {
    {"Relic of the Scholar", "+5% dmg >75% HP"},
    {"Relic of the Eagle", "+10% dmg >80% HP"},
    {"Relic of Fireworks", "Strike dmg on weapon skill"},
    {"Relic of the Thief", "Weapon skill CDR, strike dmg"},
    {"Relic of Cerus", "Elite skill +dmg, boon/condi"},
    {"Relic of Isgarren", "+strike/condi dur after evade"},
    {"Relic of the Brawler", "Strike dmg, protection, resolution"},
    {"Relic of the Claw", "Strike dmg on disable"},
    {"Relic of the Deadeye", "Strike dmg, cantrip bonus"},
    {"Relic of the Dragonhunter", "Trap dmg + condi dur"},
    {"Relic of Mount Balrior", "Elite skill +strike dmg"},
    {"Relic of Peitha", "Torment/strike on shadowstep"},
    {"Relic of Lyhr", "Heal + strike dmg"},
    {"Relic of Fire", "Fire aura on heal skill"},
    {"Relic of Dagda", "Elite skill condi transfer"},
    {"Relic of the Fractal", "Bleed, burn, torment"},
    {"Relic of Akeem", "Confusion + torment on disable"},
    {"Relic of the Aristocracy", "Condi dur, weakness, vuln"},
    {"Relic of the Afflicted", "Poison + bleed"},
    {"Relic of the Nightmare", "Elite: fear + poison"},
    {"Relic of the Krait", "Elite: conditions"},
    {"Relic of the Sorcerer", "Vuln on cripple/chill/immob"},
    {"Relic of the Mirage", "Torment on evade"},
    {"Relic of Thorns", "Condition dmg, poison"},
    {"Relic of the Blightbringer", "Poison + condi"},
    {"Relic of Nourys", "Boon strip + dmg/heal"},
    {"Relic of the Scourge", "Barrier + condi dur"},
    {"Relic of the Demon Queen", "Poison on disable"},
    {"Relic of Mosyn", "Condition on evade"},
    {"Relic of the Biomancer", "Heal + bleed/poison"},
    {"Relic of the First Revenant", "Condi dmg + resistance"},
    {"Relic of Durability", "Protection/regen/resolution on heal"},
    {"Relic of the Monk", "Healing + boon"},
    {"Relic of the Flock", "Heal on healing skill"},
    {"Relic of Castora", "Heal based on HP"},
    {"Relic of the Water", "Condi cleanse on heal skill"},
    {"Relic of the Defender", "Heal on block"},
    {"Relic of Vampirism", "Lifesteal + heal on kill"},
    {"Relic of Dwayna", "Regen"},
    {"Relic of the Centaur", "Stability on heal skill"},
    {"Relic of Mercy", "Revive speed + heal"},
    {"Relic of Nayos", "Condi + heal"},
    {"Relic of Karakosa", "Heal on blast finisher"},
    {"Relic of the Nautical Beast", "Heal + water field"},
    {"Relic of the Living City", "Titan Potential on field/disable/evade"},
    {"Relic of Leadership", "Condi cleanse + boon"},
    {"Relic of the Herald", "Concentration + boon"},
    {"Relic of the Firebrand", "Mantra + boon dur"},
    {"Relic of Rivers", "Alacrity + regen on dodge"},
    {"Relic of the Chronomancer", "Quickness on well"},
    {"Relic of the Pack", "Superspeed/might/fury on elite"},
    {"Relic of Altruism", "Might + fury on heal skill"},
    {"Relic of the Cavalier", "Aegis/quickness/swiftness on engage"},
    {"Relic of the Twin Generals", "Might + weakness on heal"},
    {"Relic of Mabon", "Might + Mabon's Strength"},
    {"Relic of the Midnight King", "Might + fury on disable"},
    {"Relic of Speed", "Movement speed + swiftness"},
    {"Relic of the Wayfinder", "Movement speed + burst speed"},
    {"Relic of Febe", "Heal skill + swiftness"},
    {"Relic of the Adventurer", "Endurance on heal skill"},
    {"Relic of Evasion", "Vigor + evade bonus"},
    {"Relic of Fog", "Dodge: glancing blows"},
    {"Relic of the Daredevil", "Dodge rolling bonuses"},
    {"Relic of Geysers", "Endurance + vigor"},
    {"Relic of Resistance", "Resistance on heal skill"},
    {"Relic of Antitoxin", "Condi cleanse"},
    {"Relic of the Trooper", "Condi cleanse on shout"},
    {"Relic of the Astral Ward", "Signet + resistance"},
    {"Relic of Reunification", "Frost/light aura on heal"},
    {"Relic of Surging", "Shocking aura on elite"},
    {"Relic of the Earth", "Protection + magnetic aura"},
    {"Relic of the Warrior", "Weapon swap CDR"},
    {"Relic of the Alliance", "Signet bonus"},
    {"Relic of Shackles", "Immobilize"},
    {"Relic of the Necromancer", "Fear"},
    {"Relic of the Scoundrel", "Blind/weakness on crit"},
    {"Relic of the Reaper", "Chill on shout"},
    {"Relic of Atrocity", "Lifesteal"},
    {"Relic of the Mist Stranger", "Siphon health"},
    {"Relic of the Ogre", "Summon rock dog"},
    {"Relic of the Lich", "Summon jagged horror"},
    {"Relic of the Golemancer", "Summon golem"},
    {"Relic of the Privateer", "Summon parrot"},
    {"Relic of the Beehive", "Elite skill summon"},
    {"Relic of Sorrow", "Elite: destroy projectiles"},
    {"Relic of the Citadel", "Elite: stun"},
    {"Relic of the Holosmith", "Elite skill bonus"},
    {"Relic of the Ice", "Elite: chill"},
    {"Relic of the Sunless", "Elite: poison/cripple"},
    {"Relic of the Wizard's Tower", "Elite: reflect + pull"},
    {"Relic of the Pirate Queen", "Quickness on disable"},
    {"Relic of the Phenom", "Cantrip/meditation + protection"},
    {"Relic of the Weaver", "Stance + strike dmg"},
    {"Relic of Bava Nisos", "Stance bonus"},
    {"Relic of the Zephyrite", "Protection + resolution"},
    {"Relic of Bloodstone", "Bloodstone blast finisher"},
    {"Relic of the Unseen Invasion", "Superspeed + stealth"},
    {"Relic of Mistburn", "WvW combat bonus"},
    {"Relic of the Forest Dweller", "Nature bonus"},
    {"Relic of the Founding", "Barrier on combo field"},
    {"Relic of the Mists Tide", "Condi on combo finisher"},
    {"Relic of the Stormsinger", "Movement skill bonus"},
    {"Relic of the Steamshrieker", "Burn on water combo"},
    {"Relic of Vass", "Heal + elixir + poison"},
    {"Relic of Zakiros", "Fury + crit dmg + healing"},
    {"Relic of the Coral Heart", "Underwater bonus"},
};
static const int RELIC_COUNT = sizeof(RELIC_LIST) / sizeof(RELIC_LIST[0]);

// Slot display name for gear panel
static const char* GearSlotDisplayName(const std::string& slot) {
    if (slot == "WeaponA1") return "Main Hand A";
    if (slot == "WeaponA2") return "Off Hand A";
    if (slot == "WeaponB1") return "Main Hand B";
    if (slot == "WeaponB2") return "Off Hand B";
    if (slot == "Accessory1") return "Accessory 1";
    if (slot == "Accessory2") return "Accessory 2";
    if (slot == "Ring1") return "Ring 1";
    if (slot == "Ring2") return "Ring 2";
    return slot.c_str();
}

static bool IsArmorSlot(const std::string& s) {
    return s == "Helm" || s == "Shoulders" || s == "Coat" ||
           s == "Gloves" || s == "Leggings" || s == "Boots";
}
static bool IsWeaponSlot(const std::string& s) {
    return s == "WeaponA1" || s == "WeaponA2" || s == "WeaponB1" || s == "WeaponB2";
}
static bool IsMainhandSlot(const std::string& s) {
    return s == "WeaponA1" || s == "WeaponB1";
}
static bool IsOffhandSlot(const std::string& s) {
    return s == "WeaponA2" || s == "WeaponB2";
}
static bool IsTrinketSlot(const std::string& s) {
    return s == "Backpack" || s == "Accessory1" || s == "Accessory2" ||
           s == "Amulet" || s == "Ring1" || s == "Ring2";
}

// Profession -> armor weight class
static int GetArmorWeight(const std::string& profession) {
    // 0=Heavy, 1=Medium, 2=Light
    if (profession == "Warrior" || profession == "Guardian" || profession == "Revenant") return 0;
    if (profession == "Ranger" || profession == "Thief" || profession == "Engineer") return 1;
    if (profession == "Elementalist" || profession == "Mesmer" || profession == "Necromancer") return 2;
    // Elite specs map to base profession via the profession field in SavedBuild
    return 0; // fallback heavy
}

// Obsidian armor item IDs [weight][slot]: weight 0=Heavy,1=Medium,2=Light, slot 0=Helm..5=Boots
static const uint32_t OBSIDIAN_ARMOR[3][6] = {
    // Heavy: Helmet, Pauldrons, Breastplate, Gauntlets, Cuisses, Greaves
    { 101544, 101551, 101521, 101609, 101568, 101460 },
    // Medium: Mask, Shoulders, Jacket, Gloves, Leggings, Boots
    { 101614, 101645, 101556, 101570, 101579, 101602 },
    // Light: Crown, Mantle, Regalia, Gloves, Pants, Shoes
    { 101516, 101462, 101499, 101536, 101501, 101535 },
};

// Gen1 legendary weapon item IDs by weapon type name
static uint32_t GetLegendaryWeaponId(const std::string& wtype) {
    if (wtype == "Axe") return 30684;           // Frostfang
    if (wtype == "Dagger") return 30687;         // Incinerator
    if (wtype == "Focus") return 30701;          // The Minstrel
    if (wtype == "Greatsword") return 30703;     // Sunrise
    if (wtype == "Hammer") return 30690;         // The Juggernaut
    if (wtype == "Longbow") return 30685;        // Kudzu
    if (wtype == "Mace") return 30692;           // The Moot
    if (wtype == "Pistol") return 30693;         // Quip
    if (wtype == "Rifle") return 30694;          // The Predator
    if (wtype == "Scepter") return 30695;        // Meteorlogicus
    if (wtype == "Shield") return 30696;         // The Flameseeker Prophecies
    if (wtype == "Shortbow") return 30686;       // The Dreamer
    if (wtype == "Staff") return 30698;          // The Bifrost
    if (wtype == "Sword") return 30699;          // Bolt
    if (wtype == "Torch") return 30700;          // Rodgort
    if (wtype == "Warhorn") return 30702;        // Howler
    return 0;
}

// Legendary trinket item IDs by slot
static uint32_t GetTrinketIconId(const std::string& slot) {
    if (slot == "Backpack") return 74155;        // Ad Infinitum
    if (slot == "Accessory1") return 81908;      // Aurora
    if (slot == "Accessory2") return 91048;      // Vision
    if (slot == "Amulet") return 95380;          // Prismatic Champion's Regalia
    if (slot == "Ring1") return 93105;           // Conflux
    if (slot == "Ring2") return 93105;           // Conflux
    if (slot == "Relic") return 101582;          // Legendary Relic
    return 0;
}

// Get the representative item ID for a build gear slot icon
static uint32_t GetGearSlotIconItemId(const AlterEgo::SavedBuild& build, const std::string& slot) {
    if (IsArmorSlot(slot)) {
        int weight = GetArmorWeight(build.profession);
        int idx = -1;
        if (slot == "Helm") idx = 0;
        else if (slot == "Shoulders") idx = 1;
        else if (slot == "Coat") idx = 2;
        else if (slot == "Gloves") idx = 3;
        else if (slot == "Leggings") idx = 4;
        else if (slot == "Boots") idx = 5;
        if (idx >= 0) return OBSIDIAN_ARMOR[weight][idx];
    } else if (IsWeaponSlot(slot)) {
        auto it = build.gear.find(slot);
        if (it != build.gear.end() && !it->second.weapon_type.empty())
            return GetLegendaryWeaponId(it->second.weapon_type);
    } else {
        return GetTrinketIconId(slot);
    }
    return 0;
}

// Open the gear customize dialog for a specific slot
static void OpenGearDialog(const std::string& buildId, const std::string& slot,
                           const AlterEgo::BuildGearSlot& current) {
    g_GearDialogOpen = true;
    g_GearDialogSlot = slot;
    g_GearDialogBuildId = buildId;
    g_GearSelectedStatId = current.stat_id;
    g_GearSelectorTab = 0;
    memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
}

// Build a sorted list of stat combos for the selection panel
static std::vector<const AlterEgo::ItemStatInfo*> GetSortedStatCombos(const char* filter) {
    std::vector<const AlterEgo::ItemStatInfo*> result;
    const auto& allStats = AlterEgo::GW2API::GetAllItemStats();
    std::string filterLower;
    if (filter && filter[0]) {
        filterLower = filter;
        for (auto& c : filterLower) c = (char)tolower(c);
    }
    for (const auto& [id, info] : allStats) {
        if (info.name.empty() || info.attributes.empty()) continue;
        if (!filterLower.empty()) {
            std::string nameLower = info.name;
            for (auto& c : nameLower) c = (char)tolower(c);
            bool matchesAttr = false;
            for (const auto& a : info.attributes) {
                std::string aLower = FriendlyAttrName(a);
                for (auto& c : aLower) c = (char)tolower(c);
                if (aLower.find(filterLower) != std::string::npos) { matchesAttr = true; break; }
            }
            if (nameLower.find(filterLower) == std::string::npos && !matchesAttr) continue;
        }
        result.push_back(&info);
    }
    std::sort(result.begin(), result.end(),
        [](const AlterEgo::ItemStatInfo* a, const AlterEgo::ItemStatInfo* b) {
            return a->name < b->name;
        });
    return result;
}

// Helper: render a card-based selector list and return the selected name (empty = no change)
// Returns true if a card was clicked (and sets outName)
static bool RenderCardSelector(const char* searchBuf, const char* childId,
                               const char* const* names, const char* const* descs, int count,
                               const std::string& currentSelection) {
    bool changed = false;
    std::string filterLower;
    if (searchBuf && searchBuf[0]) {
        filterLower = searchBuf;
        for (auto& c : filterLower) c = (char)tolower(c);
    }

    ImGui::BeginChild(childId, ImVec2(0, -30), true);
    for (int i = 0; i < count; i++) {
        if (!filterLower.empty()) {
            std::string nameLow = names[i];
            for (auto& c : nameLow) c = (char)tolower(c);
            std::string descLow = descs[i];
            for (auto& c : descLow) c = (char)tolower(c);
            if (nameLow.find(filterLower) == std::string::npos &&
                descLow.find(filterLower) == std::string::npos) continue;
        }

        ImGui::PushID(i);
        bool isSelected = (currentSelection == names[i]);

        ImVec2 cardStart = ImGui::GetCursorScreenPos();
        float cardW = ImGui::GetContentRegionAvail().x;
        float cardH = 32.0f;

        ImU32 bgCol = isSelected ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
        ImGui::GetWindowDrawList()->AddRectFilled(
            cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH), bgCol, 3.0f);
        ImGui::GetWindowDrawList()->AddRect(
            cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH),
            isSelected ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

        ImGui::SetCursorScreenPos(cardStart);
        if (ImGui::InvisibleButton("##sel", ImVec2(cardW, cardH))) {
            changed = true;
        }

        ImVec2 textPos(cardStart.x + 6, cardStart.y + 2);
        ImGui::GetWindowDrawList()->AddText(textPos,
            IM_COL32(200, 180, 255, 255), names[i]);
        textPos.y += 14.0f;
        ImGui::GetWindowDrawList()->AddText(textPos,
            IM_COL32(150, 150, 150, 220), descs[i]);
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x, cardStart.y + cardH + 2));

        ImGui::PopID();
        if (changed) {
            ImGui::EndChild();
            return true; // caller should set the name from names[i]
        }
    }
    ImGui::EndChild();
    return false;
}

// Render the Customize popup dialog
static void RenderGearCustomizeDialog() {
    if (!g_GearDialogOpen) return;

    ImGui::SetNextWindowSize(ImVec2(540, 440), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Customize##GearDialog", &g_GearDialogOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // Find the build being edited
    AlterEgo::SavedBuild* editBuild = nullptr;
    {
        auto& builds = const_cast<std::vector<AlterEgo::SavedBuild>&>(
            AlterEgo::GW2API::GetSavedBuilds());
        for (auto& b : builds) {
            if (b.id == g_GearDialogBuildId) { editBuild = &b; break; }
        }
    }
    if (!editBuild) {
        ImGui::Text("Build not found.");
        ImGui::End();
        return;
    }

    auto& gs = editBuild->gear[g_GearDialogSlot];
    gs.slot = g_GearDialogSlot;
    bool isArmor = IsArmorSlot(g_GearDialogSlot);
    bool isWeapon = IsWeaponSlot(g_GearDialogSlot);
    bool isRelic = (g_GearDialogSlot == "Relic");

    // Two-column layout: left = current item, right = selector
    if (ImGui::BeginTable("##custLayout", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("##custLeft", ImGuiTableColumnFlags_WidthFixed, 220.0f);
        ImGui::TableSetupColumn("##custRight", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        // === LEFT: Current slot info ===
        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", GearSlotDisplayName(g_GearDialogSlot));
        ImGui::Separator();

        // Weapon type area FIRST (clickable to switch to weapon type tab, weapon slots only)
        if (isWeapon) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float h = 30.0f;
            ImU32 bg = (g_GearSelectorTab == 2) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 2) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##weaponTypeArea", ImVec2(w, h))) {
                g_GearSelectorTab = 2;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (!gs.weapon_type.empty()) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Weapon: %s", WeaponDisplayName(gs.weapon_type));
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(220, 200, 120, 255), buf);
            } else {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), "Weapon: (click to select)");
            }
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
        }

        if (isRelic) {
            // Relic display area (clickable to select relic)
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float h = 44.0f;
            ImU32 bg = (g_GearSelectorTab == 0) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 0) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##relicArea", ImVec2(w, h))) {
                g_GearSelectorTab = 0;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (!editBuild->relic_name.empty()) {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(200, 180, 255, 255), editBuild->relic_name.c_str());
                // Show bonus text if we can find it in the list
                for (int i = 0; i < RELIC_COUNT; i++) {
                    if (editBuild->relic_name == RELIC_LIST[i].name) {
                        tp.y += 14.0f;
                        ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(150, 150, 150, 220), RELIC_LIST[i].bonus);
                        break;
                    }
                }
            } else {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), "Relic: (click to select)");
            }
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
        } else {
            // Stat combo (clickable to switch to stats tab)
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            // Dynamic height: stat name line + one line per attribute + padding
            int numAttrs = 0;
            if (gs.stat_id != 0) {
                const auto* si = AlterEgo::GW2API::GetItemStatInfo(gs.stat_id);
                if (si) numAttrs = (int)si->attributes.size();
            }
            float h = 6.0f + 16.0f + numAttrs * 13.0f;
            if (h < 30.0f) h = 30.0f;
            ImU32 bg = (g_GearSelectorTab == 0) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 0) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##statArea", ImVec2(w, h))) {
                g_GearSelectorTab = 0;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (gs.stat_id != 0 && !gs.stat_name.empty()) {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(255, 210, 80, 255), gs.stat_name.c_str());
                const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(gs.stat_id);
                if (statInfo) {
                    for (const auto& a : statInfo->attributes) {
                        tp.y += 13.0f;
                        char buf[64];
                        snprintf(buf, sizeof(buf), "  +%s", FriendlyAttrName(a));
                        ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(100, 200, 100, 255), buf);
                    }
                }
            } else {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), "(click to select stat)");
            }
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
        }

        // Rune/Sigil area (clickable to switch to rune/sigil tab)
        if (isArmor || isWeapon) {
            const char* upgradeLabel = isArmor ? "Rune" : "Sigil";
            const std::string& upgradeName = isArmor ? gs.rune : gs.sigil;

            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float h = 30.0f;
            ImU32 bg = (g_GearSelectorTab == 1) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 1) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##upgradeArea", ImVec2(w, h))) {
                g_GearSelectorTab = 1;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (!upgradeName.empty()) {
                char buf[256];
                snprintf(buf, sizeof(buf), "%s: %s", upgradeLabel, upgradeName.c_str());
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(200, 180, 255, 255), buf);
            } else {
                char buf[64];
                snprintf(buf, sizeof(buf), "%s: (click to select)", upgradeLabel);
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), buf);
            }
            tp.y += 14.0f;
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Apply-to-all buttons
        if (IsArmorSlot(g_GearDialogSlot) && gs.stat_id != 0) {
            if (ImGui::Button("Apply Stat to All Armor")) {
                const char* armorSlots[] = {"Helm","Shoulders","Coat","Gloves","Leggings","Boots"};
                for (const char* s : armorSlots) {
                    auto& target = editBuild->gear[s];
                    target.slot = s;
                    target.stat_id = gs.stat_id;
                    target.stat_name = gs.stat_name;
                }
                AlterEgo::GW2API::SaveBuildLibrary();
            }
        }
        if (IsArmorSlot(g_GearDialogSlot) && !gs.rune.empty()) {
            if (ImGui::Button("Apply Rune to All Armor")) {
                const char* armorSlots[] = {"Helm","Shoulders","Coat","Gloves","Leggings","Boots"};
                for (const char* s : armorSlots) {
                    auto& target = editBuild->gear[s];
                    target.slot = s;
                    target.rune = gs.rune;
                }
                editBuild->rune_name = gs.rune;
                AlterEgo::GW2API::SaveBuildLibrary();
            }
        }
        if (IsTrinketSlot(g_GearDialogSlot) && gs.stat_id != 0) {
            if (ImGui::Button("Apply Stat to All Trinkets")) {
                const char* trinketSlots[] = {"Backpack","Accessory1","Accessory2","Amulet","Ring1","Ring2"};
                for (const char* s : trinketSlots) {
                    auto& target = editBuild->gear[s];
                    target.slot = s;
                    target.stat_id = gs.stat_id;
                    target.stat_name = gs.stat_name;
                }
                AlterEgo::GW2API::SaveBuildLibrary();
            }
        }

        // === RIGHT: Selector panel ===
        ImGui::TableNextColumn();

        // Search bar (shared between tabs)
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##search", "Search...", g_GearStatSearch, sizeof(g_GearStatSearch));

        if (g_GearSelectorTab == 0 && isRelic) {
            // --- Relic selector ---
            std::string filterLower;
            if (g_GearStatSearch[0]) {
                filterLower = g_GearStatSearch;
                for (auto& c : filterLower) c = (char)tolower(c);
            }

            ImGui::BeginChild("##relicList", ImVec2(0, -30), true);
            for (int i = 0; i < RELIC_COUNT; i++) {
                if (!filterLower.empty()) {
                    std::string nLow = RELIC_LIST[i].name;
                    for (auto& c : nLow) c = (char)tolower(c);
                    std::string bLow = RELIC_LIST[i].bonus;
                    for (auto& c : bLow) c = (char)tolower(c);
                    if (nLow.find(filterLower) == std::string::npos &&
                        bLow.find(filterLower) == std::string::npos) continue;
                }

                ImGui::PushID(i);
                bool isSel = (editBuild->relic_name == RELIC_LIST[i].name);

                ImVec2 cs = ImGui::GetCursorScreenPos();
                float cw = ImGui::GetContentRegionAvail().x;
                float ch = 32.0f;
                ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                    isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                ImGui::SetCursorScreenPos(cs);
                if (ImGui::InvisibleButton("##relsel", ImVec2(cw, ch))) {
                    editBuild->relic_name = RELIC_LIST[i].name;
                    AlterEgo::GW2API::SaveBuildLibrary();
                }

                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(cs.x+6, cs.y+2), IM_COL32(200, 180, 255, 255), RELIC_LIST[i].name);
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), RELIC_LIST[i].bonus);
                ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                ImGui::PopID();
            }
            ImGui::EndChild();
        } else if (g_GearSelectorTab == 0) {
            // --- Stat combo selector ---
            ImGui::BeginChild("##statList", ImVec2(0, -30), true);
            auto combos = GetSortedStatCombos(g_GearStatSearch);
            for (const auto* stat : combos) {
                ImGui::PushID((int)stat->id);
                bool isSelected = (g_GearSelectedStatId == stat->id);

                ImVec2 cardStart = ImGui::GetCursorScreenPos();
                float cardW = ImGui::GetContentRegionAvail().x;
                float cardH = 14.0f + stat->attributes.size() * 14.0f + 6.0f;

                ImU32 bgCol = isSelected ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH), bgCol, 3.0f);
                ImGui::GetWindowDrawList()->AddRect(
                    cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH),
                    isSelected ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                ImGui::SetCursorScreenPos(cardStart);
                if (ImGui::InvisibleButton("##sel", ImVec2(cardW, cardH))) {
                    g_GearSelectedStatId = stat->id;
                    gs.stat_id = stat->id;
                    gs.stat_name = stat->name;
                    AlterEgo::GW2API::SaveBuildLibrary();
                }

                ImVec2 textPos(cardStart.x + 6, cardStart.y + 3);
                ImGui::GetWindowDrawList()->AddText(textPos,
                    IM_COL32(255, 210, 80, 255), stat->name.c_str());
                for (size_t ai = 0; ai < stat->attributes.size(); ai++) {
                    textPos.y += 14.0f;
                    char attrBuf[64];
                    snprintf(attrBuf, sizeof(attrBuf), "+%s", FriendlyAttrName(stat->attributes[ai]));
                    ImGui::GetWindowDrawList()->AddText(textPos,
                        IM_COL32(100, 200, 100, 255), attrBuf);
                }
                ImGui::SetCursorScreenPos(ImVec2(cardStart.x, cardStart.y + cardH + 2));

                ImGui::PopID();
            }
            ImGui::EndChild();
        } else if (g_GearSelectorTab == 1) {
            // --- Rune or Sigil selector ---
            std::string filterLower;
            if (g_GearStatSearch[0]) {
                filterLower = g_GearStatSearch;
                for (auto& c : filterLower) c = (char)tolower(c);
            }

            ImGui::BeginChild("##upgradeList", ImVec2(0, -30), true);

            if (isArmor) {
                for (int i = 0; i < RUNE_COUNT; i++) {
                    if (!filterLower.empty()) {
                        std::string nLow = RUNE_LIST[i].name;
                        for (auto& c : nLow) c = (char)tolower(c);
                        std::string bLow = RUNE_LIST[i].bonus;
                        for (auto& c : bLow) c = (char)tolower(c);
                        if (nLow.find(filterLower) == std::string::npos &&
                            bLow.find(filterLower) == std::string::npos) continue;
                    }

                    ImGui::PushID(i);
                    bool isSel = (gs.rune == RUNE_LIST[i].name);

                    ImVec2 cs = ImGui::GetCursorScreenPos();
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 32.0f;
                    ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                    ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                    ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                        isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                    ImGui::SetCursorScreenPos(cs);
                    if (ImGui::InvisibleButton("##rsel", ImVec2(cw, ch))) {
                        gs.rune = RUNE_LIST[i].name;
                        AlterEgo::GW2API::SaveBuildLibrary();
                    }

                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+2), IM_COL32(200, 180, 255, 255), RUNE_LIST[i].name);
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), RUNE_LIST[i].bonus);
                    ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                    ImGui::PopID();
                }
            } else if (isWeapon) {
                for (int i = 0; i < SIGIL_COUNT; i++) {
                    if (!filterLower.empty()) {
                        std::string nLow = SIGIL_LIST[i].name;
                        for (auto& c : nLow) c = (char)tolower(c);
                        std::string bLow = SIGIL_LIST[i].bonus;
                        for (auto& c : bLow) c = (char)tolower(c);
                        if (nLow.find(filterLower) == std::string::npos &&
                            bLow.find(filterLower) == std::string::npos) continue;
                    }

                    ImGui::PushID(i);
                    bool isSel = (gs.sigil == SIGIL_LIST[i].name);

                    ImVec2 cs = ImGui::GetCursorScreenPos();
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 32.0f;
                    ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                    ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                    ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                        isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                    ImGui::SetCursorScreenPos(cs);
                    if (ImGui::InvisibleButton("##ssel", ImVec2(cw, ch))) {
                        gs.sigil = SIGIL_LIST[i].name;
                        AlterEgo::GW2API::SaveBuildLibrary();
                    }

                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+2), IM_COL32(200, 180, 255, 255), SIGIL_LIST[i].name);
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), SIGIL_LIST[i].bonus);
                    ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
        } else if (g_GearSelectorTab == 2 && isWeapon) {
            // --- Weapon type selector ---
            const auto* profWeapons = AlterEgo::GW2API::GetProfessionWeapons(editBuild->profession);
            if (!profWeapons) {
                AlterEgo::GW2API::FetchProfessionPaletteAsync(editBuild->profession);
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Loading weapon data...");
            } else {
                std::string filterLower;
                if (g_GearStatSearch[0]) {
                    filterLower = g_GearStatSearch;
                    for (auto& c : filterLower) c = (char)tolower(c);
                }

                bool isMH = IsMainhandSlot(g_GearDialogSlot);
                bool isOH = IsOffhandSlot(g_GearDialogSlot);

                ImGui::BeginChild("##weaponTypeList", ImVec2(0, -30), true);
                int idx = 0;
                for (const auto& [wname, wd] : *profWeapons) {
                    // Filter by mainhand/offhand/two-hand
                    if (isMH && !wd.mainhand && !wd.two_handed) continue;
                    if (isOH && !wd.offhand) continue;

                    // Search filter
                    if (!filterLower.empty()) {
                        std::string nLow = wname;
                        for (auto& c : nLow) c = (char)tolower(c);
                        if (nLow.find(filterLower) == std::string::npos) continue;
                    }

                    ImGui::PushID(idx++);
                    bool isSel = (gs.weapon_type == wname);

                    // Build description string
                    std::string desc;
                    if (wd.two_handed) desc = "Two-Handed";
                    else if (wd.mainhand && wd.offhand) desc = "Main / Off Hand";
                    else if (wd.mainhand) desc = "Main Hand";
                    else if (wd.offhand) desc = "Off Hand";
                    if (wd.specialization != 0) {
                        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(wd.specialization);
                        if (specInfo) desc += " (" + specInfo->name + ")";
                    }

                    ImVec2 cs = ImGui::GetCursorScreenPos();
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 32.0f;
                    ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                    ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                    ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                        isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                    ImGui::SetCursorScreenPos(cs);
                    if (ImGui::InvisibleButton("##wsel", ImVec2(cw, ch))) {
                        gs.weapon_type = wname;
                        // Fetch weapon skill details (including attunements and dual wield)
                        std::vector<uint32_t> skillIds;
                        for (int si = 0; si < 5; si++) {
                            if (wd.skills[si] != 0) skillIds.push_back(wd.skills[si]);
                        }
                        for (const auto& [att, askills] : wd.attunement_skills) {
                            for (auto sid : askills) { if (sid != 0) skillIds.push_back(sid); }
                        }
                        for (const auto& [dw, sid] : wd.dual_wield) { if (sid != 0) skillIds.push_back(sid); }
                        if (!skillIds.empty())
                            AlterEgo::GW2API::FetchSkillDetailsAsync(skillIds);
                        // Fetch weapon item icon
                        uint32_t weapItemId = GetLegendaryWeaponId(wname);
                        if (weapItemId != 0)
                            AlterEgo::GW2API::FetchItemDetailsAsync({weapItemId});
                        AlterEgo::GW2API::SaveBuildLibrary();
                    }

                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+2), IM_COL32(220, 200, 120, 255), WeaponDisplayName(wname));
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), desc.c_str());
                    ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                    ImGui::PopID();
                }
                ImGui::EndChild();
            }
        }

        // Accept / Cancel buttons
        if (ImGui::Button("Accept", ImVec2(80, 0))) {
            AlterEgo::GW2API::SaveBuildLibrary();
            g_GearDialogOpen = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            g_GearDialogOpen = false;
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

// Render a single gear slot with icon for the build gear panel
static void RenderBuildGearSlot(AlterEgo::SavedBuild& build, const char* slot) {
    auto it = build.gear.find(slot);
    bool hasStat = (it != build.gear.end() && it->second.stat_id != 0);
    bool hasWeaponType = (it != build.gear.end() && !it->second.weapon_type.empty());
    bool hasData = hasStat || hasWeaponType;

    ImGui::PushID(slot);

    // Draw icon (same size as equipment panel)
    float iconSz = ICON_SIZE;
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Try to render a real item icon
    static std::set<uint32_t> s_requestedItemIds; // dedup: only fetch once
    uint32_t iconItemId = GetGearSlotIconItemId(build, slot);
    Texture_t* tex = nullptr;
    if (iconItemId != 0) {
        const auto* itemInfo = AlterEgo::GW2API::GetItemInfo(iconItemId);
        if (itemInfo && !itemInfo->icon_url.empty()) {
            AlterEgo::IconManager::RequestIcon(iconItemId, itemInfo->icon_url);
            tex = AlterEgo::IconManager::GetIcon(iconItemId);
        } else if (s_requestedItemIds.find(iconItemId) == s_requestedItemIds.end()) {
            // Fetch item details only once per item ID
            s_requestedItemIds.insert(iconItemId);
            AlterEgo::GW2API::FetchItemDetailsAsync({iconItemId});
        }
    }

    if (tex && tex->Resource) {
        // Render real icon with legendary border
        ImVec4 borderColor = hasData ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.5f, 0.3f, 0.6f, 0.8f);
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(pos.x - 1, pos.y - 1),
            ImVec2(pos.x + iconSz + 1, pos.y + iconSz + 1),
            ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, 2.0f);
        ImGui::Image(tex->Resource, ImVec2(iconSz, iconSz));
    } else {
        // Fallback: colored placeholder with slot initial
        ImU32 bgCol = hasData ? IM_COL32(50, 45, 30, 200) : IM_COL32(40, 40, 40, 180);
        ImU32 borderCol = hasData ? IM_COL32(255, 200, 60, 200) : IM_COL32(80, 80, 80, 200);
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, ImVec2(pos.x + iconSz, pos.y + iconSz), bgCol);
        ImGui::GetWindowDrawList()->AddRect(
            pos, ImVec2(pos.x + iconSz, pos.y + iconSz), borderCol, 0.0f, 0, 1.5f);

        const char* initial = slot;
        if (strcmp(slot, "Helm") == 0) initial = "H";
        else if (strcmp(slot, "Shoulders") == 0) initial = "S";
        else if (strcmp(slot, "Coat") == 0) initial = "C";
        else if (strcmp(slot, "Gloves") == 0) initial = "G";
        else if (strcmp(slot, "Leggings") == 0) initial = "L";
        else if (strcmp(slot, "Boots") == 0) initial = "B";
        else if (strcmp(slot, "WeaponA1") == 0 || strcmp(slot, "WeaponB1") == 0) initial = "M";
        else if (strcmp(slot, "WeaponA2") == 0 || strcmp(slot, "WeaponB2") == 0) initial = "O";
        else if (strcmp(slot, "Backpack") == 0) initial = "Bk";
        else if (strcmp(slot, "Accessory1") == 0 || strcmp(slot, "Accessory2") == 0) initial = "A";
        else if (strcmp(slot, "Amulet") == 0) initial = "Am";
        else if (strcmp(slot, "Ring1") == 0 || strcmp(slot, "Ring2") == 0) initial = "R";
        else if (strcmp(slot, "Relic") == 0) initial = "Re";

        ImVec2 textSz = ImGui::CalcTextSize(initial);
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(pos.x + (iconSz - textSz.x) * 0.5f, pos.y + (iconSz - textSz.y) * 0.5f),
            hasData ? IM_COL32(255, 210, 80, 200) : IM_COL32(100, 100, 100, 180), initial);
        ImGui::Dummy(ImVec2(iconSz, iconSz));
    }

    // Invisible button over icon area (overlaid on top for click handling)
    ImGui::SetCursorScreenPos(pos);
    if (ImGui::InvisibleButton("##icon", ImVec2(iconSz, iconSz))) {
        AlterEgo::BuildGearSlot current;
        if (it != build.gear.end()) current = it->second;
        current.slot = slot;
        OpenGearDialog(build.id, slot, current);
    }
    bool hovered = ImGui::IsItemHovered();

    // Name text next to icon
    bool isRelicSlot = (strcmp(slot, "Relic") == 0);
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    if (isRelicSlot) {
        if (!build.relic_name.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.8f, 1.0f), "%s", build.relic_name.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Relic");
        }
    } else if (hasStat && hasWeaponType) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s %s",
            it->second.stat_name.c_str(), WeaponDisplayName(it->second.weapon_type));
    } else if (hasStat) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s %s",
            it->second.stat_name.c_str(), GearSlotDisplayName(slot));
    } else if (hasWeaponType) {
        ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.5f, 1.0f), "%s %s",
            WeaponDisplayName(it->second.weapon_type), GearSlotDisplayName(slot));
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", GearSlotDisplayName(slot));
    }
    if (!isRelicSlot && it != build.gear.end() && !it->second.rune.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "%s", it->second.rune.c_str());
    }
    if (!isRelicSlot && it != build.gear.end() && !it->second.sigil.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "%s", it->second.sigil.c_str());
    }
    ImGui::EndGroup();

    // Tooltip
    if (hovered && isRelicSlot && !build.relic_name.empty()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.8f, 1.0f), "%s", build.relic_name.c_str());
        for (int ri = 0; ri < RELIC_COUNT; ri++) {
            if (build.relic_name == RELIC_LIST[ri].name) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", RELIC_LIST[ri].bonus);
                break;
            }
        }
        ImGui::EndTooltip();
    } else if (hovered && hasData) {
        ImGui::BeginTooltip();
        if (hasWeaponType) {
            ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.5f, 1.0f), "%s", WeaponDisplayName(it->second.weapon_type));
        }
        if (hasStat) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s %s",
                it->second.stat_name.c_str(), GearSlotDisplayName(slot));
            const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(it->second.stat_id);
            if (statInfo) {
                for (const auto& a : statInfo->attributes) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "  +%s", FriendlyAttrName(a));
                }
            }
        }
        ImGui::EndTooltip();
    } else if (hovered) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s (click to customize)", GearSlotDisplayName(slot));
        ImGui::EndTooltip();
    }

    ImGui::PopID();
}

// Render the full gear section for a saved build
static void RenderBuildGearPanel(AlterEgo::SavedBuild& build) {
    ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Gear");
    ImGui::Separator();

    if (ImGui::BeginTable("##buildGear", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("##gearLeft", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##gearRight", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        // Left column: Armor
        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Armor");
        RenderBuildGearSlot(build, "Helm");
        RenderBuildGearSlot(build, "Shoulders");
        RenderBuildGearSlot(build, "Coat");
        RenderBuildGearSlot(build, "Gloves");
        RenderBuildGearSlot(build, "Leggings");
        RenderBuildGearSlot(build, "Boots");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Weapons");
        RenderBuildGearSlot(build, "WeaponA1");
        RenderBuildGearSlot(build, "WeaponA2");
        RenderBuildGearSlot(build, "WeaponB1");
        RenderBuildGearSlot(build, "WeaponB2");

        // Right column: Trinkets + Relic + Rune
        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Trinkets");
        RenderBuildGearSlot(build, "Backpack");
        RenderBuildGearSlot(build, "Accessory1");
        RenderBuildGearSlot(build, "Accessory2");
        RenderBuildGearSlot(build, "Amulet");
        RenderBuildGearSlot(build, "Ring1");
        RenderBuildGearSlot(build, "Ring2");
        RenderBuildGearSlot(build, "Relic");

        ImGui::EndTable();
    }
}

// Resolve negative trait placeholders in a saved build once spec data is available
static void ResolveBuildTraitPlaceholders(AlterEgo::SavedBuild& build) {
    bool changed = false;
    for (int i = 0; i < 3; i++) {
        if (build.specializations[i].spec_id == 0) continue;
        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(build.specializations[i].spec_id);
        if (!specInfo || specInfo->major_traits.size() < 9) continue;
        for (int tier = 0; tier < 3; tier++) {
            if (build.specializations[i].traits[tier] >= 0) continue;
            int choice = -(build.specializations[i].traits[tier]);
            if (choice >= 1 && choice <= 3) {
                int mIdx = tier * 3 + (choice - 1);
                build.specializations[i].traits[tier] = (int)specInfo->major_traits[mIdx];
                changed = true;
            }
        }
    }
    if (changed) AlterEgo::GW2API::SaveBuildLibrary();
}

// Render the build preview panel for a saved build (reuses spec/trait rendering logic)
static void RenderSavedBuildPreview(const AlterEgo::SavedBuild& build) {
    // Try to resolve any placeholder traits (negative values from import without spec cache)
    // Safe to cast: we only mutate trait values from negative to positive
    ResolveBuildTraitPlaceholders(const_cast<AlterEgo::SavedBuild&>(build));

    ImVec4 profColor = GetProfessionColor(build.profession);
    ImGui::TextColored(profColor, "%s", build.name.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", build.profession.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.65f, 0.4f, 1.0f), "[%s]", GameModeLabel(build.game_mode));

    ImGui::Separator();

    // Specializations with trait grid (same logic as RenderBuildPanel)
    const float iconSz = TRAIT_ICON_SIZE;
    const float colW = iconSz * 2.0f;

    for (int i = 0; i < 3; i++) {
        const auto& spec = build.specializations[i];
        if (spec.spec_id == 0) {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "  (empty)");
            continue;
        }

        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(spec.spec_id);
        std::string specName = specInfo ? specInfo->name : ("Spec " + std::to_string(spec.spec_id));
        bool isElite = specInfo ? specInfo->elite : false;
        ImVec4 specColor = isElite ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

        ImGui::PushID(spec.spec_id);
        ImGui::TextColored(specColor, "%s", specName.c_str());

        if (specInfo && specInfo->major_traits.size() >= 9) {
            ImVec2 specCenter(0, 0);
            ImVec2 minorCenters[3] = {};
            ImVec2 selectedCenters[3] = {};
            bool hasSelected[3] = {false, false, false};

            char outerId[32];
            snprintf(outerId, sizeof(outerId), "##lso_%u", spec.spec_id);
            if (ImGui::BeginTable(outerId, 2,
                    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                    SPEC_PORTRAIT_SIZE + 8.0f);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                float gridH = iconSz * 3 + ImGui::GetStyle().CellPadding.y * 6;
                float padY = (gridH - SPEC_PORTRAIT_SIZE) * 0.5f;
                if (padY > 0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padY);

                if (specInfo && !specInfo->icon_url.empty()) {
                    Texture_t* tex = AlterEgo::IconManager::GetIcon(spec.spec_id);
                    if (tex && tex->Resource) {
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Image(tex->Resource,
                            ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    } else {
                        AlterEgo::IconManager::RequestIcon(spec.spec_id, specInfo->icon_url);
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Dummy(ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::TextColored(specColor, "%s", specName.c_str());
                        const auto* specDesc = AlterEgo::GetSpecDescription(specName);
                        if (specDesc) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.9f, 1.0f),
                                "\"%s\"", specDesc->flavor);
                            ImGui::Spacing();
                            ImGui::PushTextWrapPos(350.0f);
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
                                "%s", specDesc->description);
                            ImGui::PopTextWrapPos();
                        }
                        ImGui::EndTooltip();
                    }
                }

                ImGui::TableNextColumn();
                char innerId[32];
                snprintf(innerId, sizeof(innerId), "##ltr_%u", spec.spec_id);
                if (ImGui::BeginTable(innerId, 6,
                        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                    for (int c = 0; c < 6; c++)
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, colW);

                    for (int row = 0; row < 3; row++) {
                        ImGui::TableNextRow();
                        for (int tier = 0; tier < 3; tier++) {
                            ImGui::TableNextColumn();
                            if (row == 1 && tier < (int)specInfo->minor_traits.size()) {
                                minorCenters[tier] = RenderTraitIcon(
                                    specInfo->minor_traits[tier], true, true, iconSz);
                            }

                            ImGui::TableNextColumn();
                            int majorIdx = tier * 3 + row;
                            uint32_t tid = specInfo->major_traits[majorIdx];
                            bool selected;
                            if (spec.traits[tier] < 0) {
                                // Placeholder: negative = -(choice) where 1=top,2=mid,3=bot
                                int choice = -(spec.traits[tier]);
                                selected = (choice >= 1 && choice <= 3 && (choice - 1) == row);
                            } else {
                                selected = ((uint32_t)spec.traits[tier] == tid);
                            }
                            ImVec2 c = RenderTraitIcon(tid, selected, false, iconSz);
                            if (selected) {
                                selectedCenters[tier] = c;
                                hasSelected[tier] = true;
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTable();
            }

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 lineCol = IM_COL32(180, 230, 255, 180);
            float specR = SPEC_PORTRAIT_SIZE * 0.5f;
            float minorR = MINOR_TRAIT_SIZE * 0.5f;
            float majorR = iconSz * 0.5f;

            auto InsetLine = [](ImVec2 a, ImVec2 b, float rA, float rB, ImVec2& outA, ImVec2& outB) {
                float dx = b.x - a.x, dy = b.y - a.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len < rA + rB) return false;
                float nx = dx / len, ny = dy / len;
                outA = ImVec2(a.x + nx * rA, a.y + ny * rA);
                outB = ImVec2(b.x - nx * rB, b.y - ny * rB);
                return true;
            };

            ImVec2 prev = specCenter;
            float prevR = specR;
            for (int tier = 0; tier < 3; tier++) {
                if (minorCenters[tier].x > 0) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, minorCenters[tier], prevR, minorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = minorCenters[tier];
                    prevR = minorR;
                }
                if (hasSelected[tier]) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, selectedCenters[tier], prevR, majorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = selectedCenters[tier];
                    prevR = majorR;
                }
            }
        }

        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Helper lambda: get weapon skills for a weapon set from gear slots
    // Handles: Elementalist attunements (Fire default), Thief dual wield, standard MH+OH
    auto GetWeaponSkills = [&](const char* mhSlot, const char* ohSlot,
                               std::vector<uint32_t>& outSkills, std::string& outLabel) {
        outSkills.clear();
        outLabel.clear();
        const auto* profWeapons = AlterEgo::GW2API::GetProfessionWeapons(build.profession);
        if (!profWeapons) return;

        auto mhIt = build.gear.find(mhSlot);
        auto ohIt = build.gear.find(ohSlot);
        std::string mhType = (mhIt != build.gear.end()) ? mhIt->second.weapon_type : "";
        std::string ohType = (ohIt != build.gear.end()) ? ohIt->second.weapon_type : "";

        // Mainhand skills (1-3, or 1-5 for two-handed)
        if (!mhType.empty()) {
            auto wit = profWeapons->find(mhType);
            if (wit != profWeapons->end()) {
                const auto& wd = wit->second;
                int count = wd.two_handed ? 5 : 3;

                // Check for attunement skills (Elementalist) — use Fire by default
                if (!wd.attunement_skills.empty()) {
                    auto attIt = wd.attunement_skills.find("Fire");
                    if (attIt == wd.attunement_skills.end())
                        attIt = wd.attunement_skills.begin(); // fallback to first
                    for (int i = 0; i < count; i++) {
                        outSkills.push_back(attIt->second[i]);
                    }
                } else {
                    for (int i = 0; i < count; i++) {
                        outSkills.push_back(wd.skills[i]);
                    }
                }

                // Thief dual wield: replace Weapon_3 if offhand-specific variant exists
                if (!wd.dual_wield.empty() && !ohType.empty() && count >= 3) {
                    auto dwIt = wd.dual_wield.find(ohType);
                    if (dwIt != wd.dual_wield.end()) {
                        outSkills[2] = dwIt->second;
                    }
                }

                outLabel = WeaponDisplayName(mhType);
                if (wd.two_handed) { return; } // two-handed uses all 5 slots
            }
        }

        // Offhand skills (4-5)
        if (!ohType.empty()) {
            auto wit = profWeapons->find(ohType);
            if (wit != profWeapons->end()) {
                const auto& wd = wit->second;
                // Pad to 3 if mainhand didn't fill
                while (outSkills.size() < 3) outSkills.push_back(0);

                // Check for attunement skills (Elementalist offhand)
                if (!wd.attunement_skills.empty()) {
                    auto attIt = wd.attunement_skills.find("Fire");
                    if (attIt == wd.attunement_skills.end())
                        attIt = wd.attunement_skills.begin();
                    outSkills.push_back(attIt->second[3]);
                    outSkills.push_back(attIt->second[4]);
                } else {
                    outSkills.push_back(wd.skills[3]);
                    outSkills.push_back(wd.skills[4]);
                }

                if (!outLabel.empty()) outLabel += " / ";
                outLabel += WeaponDisplayName(ohType);
            }
        }
    };

    // Weapon set A skills
    std::vector<uint32_t> wpnSkillsA, wpnSkillsB;
    std::string wpnLabelA, wpnLabelB;
    GetWeaponSkills("WeaponA1", "WeaponA2", wpnSkillsA, wpnLabelA);
    GetWeaponSkills("WeaponB1", "WeaponB2", wpnSkillsB, wpnLabelB);

    // Fetch skill details for weapon skills
    {
        std::vector<uint32_t> toFetch;
        for (auto id : wpnSkillsA) { if (id && !AlterEgo::GW2API::GetSkillInfo(id)) toFetch.push_back(id); }
        for (auto id : wpnSkillsB) { if (id && !AlterEgo::GW2API::GetSkillInfo(id)) toFetch.push_back(id); }
        if (!toFetch.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(toFetch);
    }

    float skillSz = ICON_SIZE;

    // Compute heal/utility/elite offset: max weapon skill count + 1 icon gap
    int maxWpnCount = 5; // default for two-handed
    if (!wpnSkillsA.empty() && wpnSkillsA.size() > (size_t)maxWpnCount)
        maxWpnCount = (int)wpnSkillsA.size();
    if (!wpnSkillsB.empty() && wpnSkillsB.size() > (size_t)maxWpnCount)
        maxWpnCount = (int)wpnSkillsB.size();
    float utilOffset = (maxWpnCount + 1) * (skillSz + 4.0f);

    // Skills layout: weapon skills on the left, utility skills on the right
    if (!wpnSkillsA.empty() || !wpnSkillsB.empty()) {
        // Weapon Set A
        if (!wpnSkillsA.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", wpnLabelA.c_str());
            ImGui::SameLine(utilOffset);
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Heal / Utility / Elite");

            ImGui::BeginGroup();
            for (size_t i = 0; i < wpnSkillsA.size(); i++) {
                if (i > 0) ImGui::SameLine();
                RenderSkillIcon(wpnSkillsA[i], skillSz);
            }
            ImGui::EndGroup();

            ImGui::SameLine(utilOffset);
            ImGui::BeginGroup();
            RenderSkillIcon(build.terrestrial_skills.heal, skillSz);
            ImGui::SameLine();
            for (int i = 0; i < 3; i++) {
                RenderSkillIcon(build.terrestrial_skills.utilities[i], skillSz);
                ImGui::SameLine();
            }
            RenderSkillIcon(build.terrestrial_skills.elite, skillSz);
            ImGui::EndGroup();
        } else {
            // No weapon set A, just show utility skills
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Heal / Utility / Elite");
            ImGui::BeginGroup();
            RenderSkillIcon(build.terrestrial_skills.heal, skillSz);
            ImGui::SameLine();
            for (int i = 0; i < 3; i++) {
                RenderSkillIcon(build.terrestrial_skills.utilities[i], skillSz);
                ImGui::SameLine();
            }
            RenderSkillIcon(build.terrestrial_skills.elite, skillSz);
            ImGui::EndGroup();
        }

        // Weapon Set B
        if (!wpnSkillsB.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", wpnLabelB.c_str());
            ImGui::BeginGroup();
            for (size_t i = 0; i < wpnSkillsB.size(); i++) {
                if (i > 0) ImGui::SameLine();
                RenderSkillIcon(wpnSkillsB[i], skillSz);
            }
            ImGui::EndGroup();
        }
    } else {
        // Fallback: no weapon types selected, just show utility skills
        ImGui::Text("Skills");
        ImGui::BeginGroup();
        RenderSkillIcon(build.terrestrial_skills.heal, skillSz);
        ImGui::SameLine();
        for (int i = 0; i < 3; i++) {
            RenderSkillIcon(build.terrestrial_skills.utilities[i], skillSz);
            ImGui::SameLine();
        }
        RenderSkillIcon(build.terrestrial_skills.elite, skillSz);
        ImGui::EndGroup();

        // Legacy weapons from chat link
        if (!build.weapons.empty()) {
            ImGui::Spacing();
            ImGui::Text("Weapons:");
            ImGui::SameLine();
            std::string wpnStr;
            for (size_t i = 0; i < build.weapons.size(); i++) {
                if (i > 0) wpnStr += ", ";
                wpnStr += AlterEgo::ChatLink::WeaponName((uint16_t)build.weapons[i]);
            }
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", wpnStr.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Gear section (clickable slots to customize stats/runes/sigils)
    RenderBuildGearPanel(const_cast<AlterEgo::SavedBuild&>(build));

    ImGui::Spacing();
    ImGui::Separator();

    // Action buttons
    if (ImGui::Button("Copy Chat Link")) {
        CopyToClipboard(build.chat_link);
        if (APIDefs) APIDefs->GUI_SendAlert("Build link copied to clipboard!");
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("Delete")) {
        AlterEgo::GW2API::RemoveSavedBuild(build.id);
        g_LibSelectedIdx = -1;
    }
    ImGui::PopStyleColor(2);

    if (!build.notes.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Notes: %s", build.notes.c_str());
    }
}

// Render the full Build Library tab
static void RenderBuildLibrary() {
    const auto& builds = AlterEgo::GW2API::GetSavedBuilds();

    // Import toggle button
    if (ImGui::Button(g_LibShowImport ? "Cancel Import" : "+ Import Build")) {
        g_LibShowImport = !g_LibShowImport;
        g_LibImportBuf[0] = '\0';
        g_LibImportName[0] = '\0';
        g_LibImportMode = 0;
        g_LibImportError.clear();
    }

    // Import panel
    if (g_LibShowImport) {
        ImGui::Separator();
        ImGui::Text("Paste build template chat link:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##import_link", g_LibImportBuf, sizeof(g_LibImportBuf));

        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("##import_name", g_LibImportName, sizeof(g_LibImportName));
        ImGui::SameLine();
        ImGui::Text("Mode:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::Combo("##import_mode", &g_LibImportMode, GameModeImportNames,
                      IM_ARRAYSIZE(GameModeImportNames));

        ImGui::SameLine();
        if (ImGui::Button("Import")) {
            std::string link(g_LibImportBuf);
            std::string name(g_LibImportName);
            if (link.empty()) {
                g_LibImportError = "Paste a chat link first.";
            } else if (name.empty()) {
                g_LibImportError = "Enter a build name.";
            } else {
                // Detect link type
                auto linkType = AlterEgo::ChatLink::DetectType(link);
                if (linkType != AlterEgo::LINK_BUILD) {
                    g_LibImportError = "Not a build template link.";
                } else {
                    // First decode to get profession, then ensure palette data
                    AlterEgo::DecodedBuildLink raw;
                    if (!AlterEgo::ChatLink::DecodeBuild(link, raw)) {
                        g_LibImportError = "Failed to decode build link.";
                    } else {
                        std::string prof = ProfessionFromCode(raw.profession);
                        if (prof == "Unknown") {
                            g_LibImportError = "Unknown profession in link.";
                        } else {
                            // Ensure palette data is available
                            if (!AlterEgo::GW2API::HasPaletteData(prof)) {
                                AlterEgo::GW2API::FetchProfessionPaletteAsync(prof);
                                g_LibImportError = "Loading " + prof + " skill data... try again in a moment.";
                            } else {
                                AlterEgo::SavedBuild build;
                                if (DecodeBuildLink(link, name,
                                        GameModeFromIndex(g_LibImportMode), build)) {
                                    AlterEgo::GW2API::AddSavedBuild(std::move(build));
                                    g_LibShowImport = false;
                                    g_LibImportBuf[0] = '\0';
                                    g_LibImportName[0] = '\0';
                                    g_LibImportError.clear();
                                    g_LibSelectedIdx = (int)AlterEgo::GW2API::GetSavedBuilds().size() - 1;
                                    g_LibDetailsFetched = false;
                                    if (APIDefs) APIDefs->GUI_SendAlert("Build imported!");
                                } else {
                                    g_LibImportError = "Failed to decode build.";
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!g_LibImportError.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", g_LibImportError.c_str());
        }
        ImGui::Separator();
    }

    if (builds.empty() && !g_LibShowImport) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
            "No saved builds. Click '+ Import Build' to add one from a chat link.");
        return;
    }

    ImGui::Spacing();

    // Filter + Search
    ImGui::SetNextItemWidth(80.0f);
    ImGui::Combo("##lib_filter", &g_LibFilterMode, GameModeNames, IM_ARRAYSIZE(GameModeNames));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputTextWithHint("##lib_search", "Search...", g_LibSearchBuf, sizeof(g_LibSearchBuf));

    // Left panel: build list, Right panel: preview
    float listWidth = 220.0f;

    ImGui::BeginChild("LibList", ImVec2(listWidth, 0), true);
    {
        struct LibItemRect { float yMin, yMax; int buildIdx; };
        std::vector<LibItemRect> libItemRects;

        // Group by profession
        std::string lastProf;
        for (int i = 0; i < (int)builds.size(); i++) {
            const auto& b = builds[i];

            // Apply filter
            if (!GameModeMatchesFilter(b.game_mode, g_LibFilterMode)) continue;

            // Apply search
            if (g_LibSearchBuf[0] != '\0') {
                std::string search(g_LibSearchBuf);
                std::string nameLower = b.name;
                std::transform(search.begin(), search.end(), search.begin(), ::tolower);
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                if (nameLower.find(search) == std::string::npos) continue;
            }

            // Profession group header
            if (b.profession != lastProf) {
                if (!lastProf.empty()) ImGui::Spacing();
                ImVec4 profColor = GetProfessionColor(b.profession);
                ImGui::TextColored(profColor, "%s", b.profession.c_str());
                ImGui::Separator();
                lastProf = b.profession;
            }

            ImGui::PushID(i);
            bool selected = (g_LibSelectedIdx == i);
            if (ImGui::Selectable("##build", selected, 0, ImVec2(0, 32))) {
                g_LibSelectedIdx = i;
                g_LibDetailsFetched = false;
            }

            // Record rect for drag-and-drop
            ImVec2 rMin = ImGui::GetItemRectMin();
            ImVec2 rMax = ImGui::GetItemRectMax();
            libItemRects.push_back({ rMin.y, rMax.y, i });

            // Drag source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                g_LibDragIdx = i;
                ImGui::SetDragDropPayload("BUILD_REORDER", &i, sizeof(int));
                ImGui::Text("%s", b.name.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::SameLine(4);
            ImGui::BeginGroup();
            ImGui::Text("%s", b.name.c_str());
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", GameModeLabel(b.game_mode));
            ImGui::EndGroup();
            ImGui::PopID();
        }

        // Draw insertion line and handle drop
        if (g_LibDragIdx >= 0 && ImGui::GetDragDropPayload() != nullptr) {
            float mouseY = ImGui::GetMousePos().y;
            int insertVisIdx = (int)libItemRects.size();
            float bestLineY = 0;

            for (int vi = 0; vi < (int)libItemRects.size(); vi++) {
                float midY = (libItemRects[vi].yMin + libItemRects[vi].yMax) * 0.5f;
                if (mouseY < midY) {
                    insertVisIdx = vi;
                    bestLineY = libItemRects[vi].yMin;
                    break;
                }
            }
            if (insertVisIdx == (int)libItemRects.size() && !libItemRects.empty())
                bestLineY = libItemRects.back().yMax;

            ImDrawList* dl = ImGui::GetWindowDrawList();
            float xMin = ImGui::GetWindowPos().x + 2;
            float xMax = xMin + ImGui::GetWindowContentRegionMax().x - 4;
            dl->AddLine(ImVec2(xMin, bestLineY), ImVec2(xMax, bestLineY),
                IM_COL32(100, 180, 255, 220), 2.0f);

            if (ImGui::IsMouseReleased(0)) {
                int targetBuildIdx = (insertVisIdx < (int)libItemRects.size())
                    ? libItemRects[insertVisIdx].buildIdx : (int)builds.size();
                if (g_LibDragIdx != targetBuildIdx) {
                    AlterEgo::GW2API::ReorderSavedBuild(g_LibDragIdx, targetBuildIdx);
                    // Fix selection to follow the moved build
                    if (g_LibSelectedIdx == g_LibDragIdx) {
                        int newIdx = (g_LibDragIdx < targetBuildIdx) ? targetBuildIdx - 1 : targetBuildIdx;
                        g_LibSelectedIdx = newIdx;
                    }
                }
                g_LibDragIdx = -1;
            }
        } else if (ImGui::IsMouseReleased(0)) {
            g_LibDragIdx = -1;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Right panel: preview
    ImGui::BeginChild("LibDetail", ImVec2(0, 0), true);
    if (g_LibSelectedIdx >= 0 && g_LibSelectedIdx < (int)builds.size()) {
        const auto& build = builds[g_LibSelectedIdx];

        // Fetch details on-demand
        if (!g_LibDetailsFetched) {
            FetchDetailsForSavedBuild(build);
            // Ensure palette data for this profession
            if (!AlterEgo::GW2API::HasPaletteData(build.profession)) {
                AlterEgo::GW2API::FetchProfessionPaletteAsync(build.profession);
            }
            g_LibDetailsFetched = true;
        }

        // Inline rename + notes editing
        if (g_LibEditBuildId != build.id) {
            g_LibEditBuildId = build.id;
            strncpy(g_LibEditName, build.name.c_str(), sizeof(g_LibEditName) - 1);
            g_LibEditName[sizeof(g_LibEditName) - 1] = '\0';
            strncpy(g_LibEditNotes, build.notes.c_str(), sizeof(g_LibEditNotes) - 1);
            g_LibEditNotes[sizeof(g_LibEditNotes) - 1] = '\0';
        }

        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(250.0f);
        if (ImGui::InputText("##edit_name", g_LibEditName, sizeof(g_LibEditName),
                ImGuiInputTextFlags_EnterReturnsTrue)) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName, g_LibEditNotes);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName, g_LibEditNotes);
        }

        ImGui::Text("Notes:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputTextMultiline("##edit_notes", g_LibEditNotes, sizeof(g_LibEditNotes),
                ImVec2(0, 60))) {
            // live typing — save handled on deactivate
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName, g_LibEditNotes);
        }

        ImGui::Separator();

        RenderSavedBuildPreview(build);
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a build from the list.");
    }
    ImGui::EndChild();
}

// --- Addon Lifecycle ---

void AddonLoad(AddonAPI_t* aApi) {
    APIDefs = aApi;
    ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
    ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))APIDefs->ImguiMalloc,
                                 (void(*)(void*, void*))APIDefs->ImguiFree);

    // Initialize subsystems
    AlterEgo::GW2API::Initialize(APIDefs);
    AlterEgo::IconManager::Initialize(APIDefs);

    // Load cached character data and build library from disk
    AlterEgo::GW2API::LoadCharacterData();
    AlterEgo::GW2API::LoadBuildLibrary();
    AlterEgo::GW2API::FetchAllItemStatsAsync();
    LoadCharSortConfig();
    RebuildCharDisplayOrder();

    // Subscribe to MumbleLink identity for current character indicator
    APIDefs->Events_Subscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

    // Subscribe to H&S events
    APIDefs->Events_Subscribe(EV_HOARD_PONG, AlterEgo::GW2API::OnHoardPong);
    APIDefs->Events_Subscribe(EV_HOARD_DATA_UPDATED, AlterEgo::GW2API::OnHoardDataUpdated);
    APIDefs->Events_Subscribe(EV_AE_CHAR_LIST_RESP, AlterEgo::GW2API::OnCharListResponse);
    APIDefs->Events_Subscribe(EV_AE_CHAR_DATA_RESP, AlterEgo::GW2API::OnCharDataResponse);
    APIDefs->Events_Subscribe(EV_AE_SKIN_UNLOCK_RESP, AlterEgo::GW2API::OnSkinUnlocksResponse);
    APIDefs->Events_Subscribe(EV_AE_ITEM_LOC_RESP, AlterEgo::GW2API::OnItemLocationResponse);
    APIDefs->Events_Subscribe(EV_AE_CLEARS_ACH_RESPONSE, OnClearsAchResponse);

    // Register render functions
    APIDefs->GUI_Register(RT_Render, AddonRender);
    APIDefs->GUI_Register(RT_OptionsRender, AddonOptions);

    // Register keybind
    APIDefs->InputBinds_RegisterWithString("KB_ALTER_EGO_TOGGLE", ProcessKeybind, "CTRL+SHIFT+E");

    // Load icon textures from embedded PNG data
    APIDefs->Textures_LoadFromMemory(TEX_ICON, (void*)ICON_NORMAL, ICON_NORMAL_size, nullptr);
    APIDefs->Textures_LoadFromMemory(TEX_ICON_HOVER, (void*)ICON_HOVER, ICON_HOVER_size, nullptr);

    // Load settings and cached data
    LoadSettings();
    LoadClearsCache();

    // Register quick access shortcut
    if (g_ShowQAIcon) {
        APIDefs->QuickAccess_Add(QA_ID, TEX_ICON, TEX_ICON_HOVER, "KB_ALTER_EGO_TOGGLE", "Alter Ego");
    }

    // Register close-on-escape
    APIDefs->GUI_RegisterCloseOnEscape("Alter Ego", &g_WindowVisible);

    // Start fetching dye colors and profession icons (public endpoints, no auth needed)
    AlterEgo::GW2API::FetchDyeColorsAsync();
    static const char* allProfs[] = {
        "Guardian", "Warrior", "Engineer", "Ranger", "Thief",
        "Elementalist", "Mesmer", "Necromancer", "Revenant"
    };
    for (const char* p : allProfs)
        AlterEgo::GW2API::FetchProfessionInfoAsync(p);

    // Ping H&S to check availability
    AlterEgo::GW2API::PingHoard();

    // Initialize Skinventory subsystems
    {
        std::string dataDir = AlterEgo::GW2API::GetDataDirectory();
        std::string skinDataDir = dataDir + "\\Skinventory";
        try { std::filesystem::create_directories(skinDataDir); } catch (...) {}
        std::string wikiCacheDir = skinDataDir + "\\wiki_cache";

        Skinventory::SkinCache::SetDataDirectory(skinDataDir);
        Skinventory::Commerce::SetDataDirectory(skinDataDir);
        Skinventory::OwnedSkins::SetDataDirectory(skinDataDir);
        Skinventory::OwnedSkins::Initialize(APIDefs);
        Skinventory::WikiImage::Initialize(APIDefs, wikiCacheDir);

        if (Skinventory::SkinCache::LoadFromDisk()) {
            Skinventory::SkinCache::UpdateCacheAsync();
        } else {
            Skinventory::SkinCache::FetchAllSkinsAsync();
        }

        if (!Skinventory::Commerce::LoadItemMap()) {
            Skinventory::Commerce::BuildItemMapAsync();
        }

        // Ping H&S so Skinventory's OwnedSkins receives the pong
        // (must be after Initialize which subscribes to EV_HOARD_PONG)
        Skinventory::OwnedSkins::PingHoardAndSeek();

        g_SkinInitialized = true;
    }

    APIDefs->Log(LOGL_INFO, "AlterEgo", "Addon loaded successfully");
}

void AddonUnload() {
    // Unsubscribe MumbleLink identity
    APIDefs->Events_Unsubscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

    // Unsubscribe H&S events
    APIDefs->Events_Unsubscribe(EV_HOARD_PONG, AlterEgo::GW2API::OnHoardPong);
    APIDefs->Events_Unsubscribe(EV_HOARD_DATA_UPDATED, AlterEgo::GW2API::OnHoardDataUpdated);
    APIDefs->Events_Unsubscribe(EV_AE_CHAR_LIST_RESP, AlterEgo::GW2API::OnCharListResponse);
    APIDefs->Events_Unsubscribe(EV_AE_CHAR_DATA_RESP, AlterEgo::GW2API::OnCharDataResponse);
    APIDefs->Events_Unsubscribe(EV_AE_SKIN_UNLOCK_RESP, AlterEgo::GW2API::OnSkinUnlocksResponse);
    APIDefs->Events_Unsubscribe(EV_AE_ITEM_LOC_RESP, AlterEgo::GW2API::OnItemLocationResponse);
    APIDefs->Events_Unsubscribe(EV_AE_CLEARS_ACH_RESPONSE, OnClearsAchResponse);

    // Shutdown Skinventory subsystems
    Skinventory::WikiImage::Shutdown();
    Skinventory::OwnedSkins::Shutdown();

    AlterEgo::IconManager::Shutdown();
    AlterEgo::GW2API::Shutdown();

    APIDefs->GUI_DeregisterCloseOnEscape("Alter Ego");
    APIDefs->QuickAccess_Remove(QA_ID);
    APIDefs->GUI_Deregister(AddonOptions);
    APIDefs->GUI_Deregister(AddonRender);

    SaveSettings();
    APIDefs = nullptr;
}

void OnMumbleIdentityUpdated(void* eventArgs) {
    if (!eventArgs) return;
    const MumbleIdentity* id = (const MumbleIdentity*)eventArgs;
    g_CurrentCharName = std::string(id->Name);
}

void ProcessKeybind(const char* aIdentifier, bool aIsRelease) {
    if (aIsRelease) return;

    if (strcmp(aIdentifier, "KB_ALTER_EGO_TOGGLE") == 0) {
        g_WindowVisible = !g_WindowVisible;
        if (APIDefs) {
            APIDefs->Log(LOGL_INFO, "AlterEgo",
                g_WindowVisible ? "Window shown" : "Window hidden");
        }
    }
}

// =========================================================================
// Clears - Backend
// =========================================================================

// Calculate the most recent daily reset (00:00 UTC) before 'now'
static std::chrono::system_clock::time_point CalcLastDailyReset(std::chrono::system_clock::time_point now) {
    auto tt = std::chrono::system_clock::to_time_t(now);
    struct tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    utc.tm_hour = 0; utc.tm_min = 0; utc.tm_sec = 0;
    auto reset = std::chrono::system_clock::from_time_t(
#ifdef _WIN32
        _mkgmtime(&utc)
#else
        timegm(&utc)
#endif
    );
    if (reset > now) reset -= std::chrono::hours(24);
    return reset;
}

// Calculate the most recent weekly reset (Monday 07:30 UTC) before 'now'
static std::chrono::system_clock::time_point CalcLastWeeklyReset(std::chrono::system_clock::time_point now) {
    auto tt = std::chrono::system_clock::to_time_t(now);
    struct tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    // tm_wday: 0=Sun, 1=Mon, ...
    int daysSinceMonday = (utc.tm_wday + 6) % 7; // Mon=0, Tue=1, ... Sun=6
    utc.tm_hour = 7; utc.tm_min = 30; utc.tm_sec = 0;
    utc.tm_mday -= daysSinceMonday;
    auto reset = std::chrono::system_clock::from_time_t(
#ifdef _WIN32
        _mkgmtime(&utc)
#else
        timegm(&utc)
#endif
    );
    if (reset > now) reset -= std::chrono::hours(24 * 7);
    return reset;
}

// Format time until next reset as human-readable string
static std::string FormatTimeUntilReset(std::chrono::system_clock::time_point resetTime,
                                         std::chrono::hours period) {
    auto now = std::chrono::system_clock::now();
    auto nextReset = resetTime + period;
    if (nextReset <= now) return "now";
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(nextReset - now).count();
    int h = (int)(remaining / 3600);
    int m = (int)((remaining % 3600) / 60);
    if (h > 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%dh %dm", h, m);
        return buf;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%dm", m);
    return buf;
}

// Parse tier from fractal achievement name
static std::string ParseFractalTier(const std::string& name) {
    if (name.find("Tier 4") != std::string::npos) return "T4";
    if (name.find("Tier 3") != std::string::npos) return "T3";
    if (name.find("Tier 2") != std::string::npos) return "T2";
    if (name.find("Tier 1") != std::string::npos) return "T1";
    if (name.find("Recommended") != std::string::npos) return "Rec";
    return "";
}

// Send H&S achievement query for all currently tracked clears IDs
static void SendClearsAchQuery() {
    std::vector<uint32_t> allIds;
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        for (const auto& e : g_DailyFractals) allIds.push_back(e.id);
        for (const auto& e : g_DailyBounties) allIds.push_back(e.id);
        for (const auto& e : g_WeeklyWings)   allIds.push_back(e.id);
        if (g_WeeklyStrikes.id > 0) allIds.push_back(g_WeeklyStrikes.id);
    }
    if (allIds.empty() || !APIDefs) return;

    HoardQueryAchievementRequest req{};
    req.api_version = HOARD_API_VERSION;
    strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
    strncpy(req.response_event, EV_AE_CLEARS_ACH_RESPONSE, sizeof(req.response_event) - 1);
    req.id_count = (uint32_t)std::min(allIds.size(), (size_t)200);
    for (uint32_t i = 0; i < req.id_count; i++) {
        req.ids[i] = allIds[i];
    }
    APIDefs->Events_Raise(EV_HOARD_QUERY_ACHIEVEMENT, &req);
}

// H&S achievement response handler for all clears
static void OnClearsAchResponse(void* eventArgs) {
    if (!eventArgs) return;
    auto* resp = (HoardQueryAchievementResponse*)eventArgs;
    if (resp->api_version != HOARD_API_VERSION) { delete resp; return; }

    if (resp->status != HOARD_STATUS_OK) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        if (resp->status == HOARD_STATUS_PENDING) {
            g_ClearsStatusMsg = "Waiting for H&S permission...";
            // Retry after a delay — user is being prompted by H&S
            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                SendClearsAchQuery();
            }).detach();
        } else {
            if (resp->status == HOARD_STATUS_DENIED)
                g_ClearsStatusMsg = "H&S permission denied";
            else
                g_ClearsStatusMsg = "H&S error";
            g_ClearsFetching = false;
        }
        delete resp;
        return;
    }

    // Build lookup maps from response
    struct AchData {
        bool done;
        int32_t current;
        int32_t max;
        std::set<uint32_t> bits;
    };
    std::unordered_map<uint32_t, AchData> achMap;
    for (uint32_t i = 0; i < resp->entry_count; i++) {
        auto& e = resp->entries[i];
        AchData ad;
        ad.done = e.done;
        ad.current = e.current;
        ad.max = e.max;
        for (uint32_t b = 0; b < e.bit_count && b < 64; b++) {
            ad.bits.insert(e.bits[b]);
        }
        achMap[e.id] = std::move(ad);
    }

    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);

        // Apply to all tracked entries
        auto applyTo = [&](ClearEntry& ce) {
            auto it = achMap.find(ce.id);
            if (it != achMap.end()) {
                ce.done = it->second.done;
                ce.current = it->second.current;
                ce.max = it->second.max;
                ce.bitDone.assign(ce.bitNames.size(), false);
                for (uint32_t bitIdx : it->second.bits) {
                    if (bitIdx < ce.bitDone.size()) {
                        ce.bitDone[bitIdx] = true;
                    }
                }
            }
        };

        for (auto& e : g_DailyFractals)  applyTo(e);
        for (auto& e : g_DailyBounties)  applyTo(e);
        for (auto& e : g_WeeklyWings)    applyTo(e);
        applyTo(g_WeeklyStrikes);

        g_ClearsFetched = true;
        g_ClearsFetching = false;
        g_ClearsStatusMsg = "";
        g_LastClearsCompletionQuery = std::chrono::steady_clock::now();
    }

    // Persist to disk
    SaveClearsCache();

    delete resp;
}

// Helper: fetch a category's achievement IDs from public API
static std::vector<uint32_t> FetchCategoryIds(uint32_t catId) {
    std::string url = "https://api.guildwars2.com/v2/achievements/categories/" + std::to_string(catId);
    std::string json = Skinventory::HttpClient::Get(url);
    std::vector<uint32_t> ids;
    try {
        auto j = nlohmann::json::parse(json);
        if (j.contains("achievements") && j["achievements"].is_array()) {
            for (const auto& id : j["achievements"]) {
                ids.push_back(id.get<uint32_t>());
            }
        }
    } catch (...) {}
    return ids;
}

// Helper: fetch achievement details (name, bits) from public API
static std::vector<ClearEntry> FetchAchievementDetails(const std::vector<uint32_t>& ids) {
    std::vector<ClearEntry> entries;
    if (ids.empty()) return entries;

    std::string idStr;
    for (size_t i = 0; i < ids.size(); i++) {
        if (i > 0) idStr += ",";
        idStr += std::to_string(ids[i]);
    }
    std::string json = Skinventory::HttpClient::Get(
        "https://api.guildwars2.com/v2/achievements?ids=" + idStr);
    if (json.empty()) return entries;

    try {
        auto j = nlohmann::json::parse(json);
        if (j.is_array()) {
            for (const auto& ach : j) {
                ClearEntry ce;
                ce.id = ach.value("id", 0u);
                ce.name = ach.value("name", "");
                if (ce.id == 0 || ce.name.empty()) continue;

                // Parse bits (sub-objectives)
                if (ach.contains("bits") && ach["bits"].is_array()) {
                    for (const auto& bit : ach["bits"]) {
                        ce.bitNames.push_back(bit.value("text", ""));
                    }
                }

                // Parse tier count from tiers
                if (ach.contains("tiers") && ach["tiers"].is_array()) {
                    auto& tiers = ach["tiers"];
                    if (!tiers.empty()) {
                        ce.max = tiers.back().value("count", 0);
                    }
                }

                entries.push_back(std::move(ce));
            }
        }
    } catch (...) {}
    return entries;
}

// Fetch all clears data (runs on background thread)
static void FetchClears() {
    if (g_ClearsFetching) return;
    g_ClearsFetching = true;
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_ClearsStatusMsg = "Fetching achievement data...";
        g_ClearsFetched = false;
    }

    std::thread([]() {
        // Step 1: Fetch all category achievement IDs
        auto fractalIds = FetchCategoryIds(CAT_DAILY_FRACTALS);
        auto bountyIds  = FetchCategoryIds(CAT_DAILY_BOUNTIES);
        auto weeklyIds  = FetchCategoryIds(CAT_WEEKLY_RAIDS);

        if (fractalIds.empty() && bountyIds.empty() && weeklyIds.empty()) {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_ClearsStatusMsg = "Failed to fetch achievement categories";
            g_ClearsFetching = false;
            return;
        }

        // Step 2: Fetch all achievement details
        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_ClearsStatusMsg = "Fetching achievement details...";
        }

        auto fractalEntries = FetchAchievementDetails(fractalIds);
        for (auto& e : fractalEntries) {
            e.tier = ParseFractalTier(e.name);
        }

        auto bountyEntries = FetchAchievementDetails(bountyIds);
        auto weeklyEntries = FetchAchievementDetails(weeklyIds);

        // Separate weekly entries into wings vs strikes meta
        ClearEntry strikesEntry{};
        std::vector<ClearEntry> wingEntries;
        for (auto& e : weeklyEntries) {
            if (e.id == ACH_WEEKLY_STRIKES) {
                strikesEntry = std::move(e);
            } else if (!e.bitNames.empty()) {
                // Per-wing achievements have bits for encounters
                wingEntries.push_back(std::move(e));
            }
        }

        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_DailyFractals = std::move(fractalEntries);
            g_DailyBounties = std::move(bountyEntries);
            g_WeeklyWings = std::move(wingEntries);
            g_WeeklyStrikes = std::move(strikesEntry);
            g_ClearsStatusMsg = "Querying completion...";
        }

        // Step 3: Query H&S for completion
        SendClearsAchQuery();
    }).detach();
}

// =========================================================================
// Skinventory UI
// =========================================================================

static ImVec4 GetSkinRarityColor(const std::string& rarity) {
    if (rarity == "Junk")       return ImVec4(0.67f, 0.67f, 0.67f, 1.0f);
    if (rarity == "Basic")      return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    if (rarity == "Fine")       return ImVec4(0.38f, 0.58f, 1.0f, 1.0f);
    if (rarity == "Masterwork") return ImVec4(0.12f, 0.72f, 0.12f, 1.0f);
    if (rarity == "Rare")       return ImVec4(0.98f, 0.82f, 0.0f, 1.0f);
    if (rarity == "Exotic")     return ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
    if (rarity == "Ascended")   return ImVec4(0.98f, 0.35f, 0.56f, 1.0f);
    if (rarity == "Legendary")  return ImVec4(0.63f, 0.21f, 0.93f, 1.0f);
    return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
}

static const ImVec4 COIN_GOLD   = ImVec4(0.85f, 0.75f, 0.10f, 1.0f);
static const ImVec4 COIN_SILVER = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
static const ImVec4 COIN_COPPER = ImVec4(0.72f, 0.45f, 0.20f, 1.0f);

static void DrawCoinIcon(ImVec4 color, float radius) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float y_center = pos.y + ImGui::GetTextLineHeight() * 0.5f;
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(pos.x + radius, y_center), radius,
        ImGui::GetColorU32(color));
    ImGui::GetWindowDrawList()->AddCircle(
        ImVec2(pos.x + radius, y_center), radius,
        ImGui::GetColorU32(ImVec4(color.x * 0.5f, color.y * 0.5f, color.z * 0.5f, 1.0f)));
    ImGui::Dummy(ImVec2(radius * 2.0f + 1.0f, ImGui::GetTextLineHeight()));
}

static void RenderCoins(int copper) {
    if (copper <= 0) {
        ImGui::TextColored(COIN_COPPER, "0");
        ImGui::SameLine(0, 2);
        DrawCoinIcon(COIN_COPPER, 5.0f);
        return;
    }
    int gold = copper / 10000;
    int silver = (copper % 10000) / 100;
    int cop = copper % 100;

    bool needSame = false;
    if (gold > 0) {
        ImGui::TextColored(COIN_GOLD, "%d", gold);
        ImGui::SameLine(0, 1);
        DrawCoinIcon(COIN_GOLD, 5.0f);
        needSame = true;
    }
    if (silver > 0 || gold > 0) {
        if (needSame) ImGui::SameLine(0, 2);
        ImGui::TextColored(COIN_SILVER, "%d", silver);
        ImGui::SameLine(0, 1);
        DrawCoinIcon(COIN_SILVER, 5.0f);
        needSame = true;
    }
    if (needSame) ImGui::SameLine(0, 2);
    ImGui::TextColored(COIN_COPPER, "%d", cop);
    ImGui::SameLine(0, 1);
    DrawCoinIcon(COIN_COPPER, 5.0f);
}

static void RenderSkinCategoryNav() {
    ImGui::BeginChild("CategoryNav", ImVec2(180, 0), true);

    auto drawBullet = [](ImVec4 color) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float y_center = pos.y + ImGui::GetTextLineHeight() * 0.5f;
        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(pos.x + 4.0f, y_center), 4.0f,
            ImGui::GetColorU32(color));
        ImGui::Dummy(ImVec2(12.0f, ImGui::GetTextLineHeight()));
        ImGui::SameLine(0, 2);
    };

    ImGui::Text("Category");
    ImGui::Separator();

    drawBullet(ImVec4(0.45f, 0.65f, 0.85f, 1.0f));
    if (ImGui::Selectable("Armor", g_SkinSelectedType == "Armor")) {
        g_SkinSelectedType = "Armor";
        g_SkinSelectedWeightClass = "Heavy";
        g_SkinSelectedSubtype = "Helm";
        g_SkinSelectedId = 0;
    }
    drawBullet(ImVec4(0.85f, 0.45f, 0.35f, 1.0f));
    if (ImGui::Selectable("Weapons", g_SkinSelectedType == "Weapon")) {
        g_SkinSelectedType = "Weapon";
        g_SkinSelectedWeightClass = "";
        g_SkinSelectedSubtype = "Axe";
        g_SkinSelectedId = 0;
    }
    drawBullet(ImVec4(0.65f, 0.50f, 0.80f, 1.0f));
    if (ImGui::Selectable("Back", g_SkinSelectedType == "Back")) {
        g_SkinSelectedType = "Back";
        g_SkinSelectedWeightClass = "";
        g_SkinSelectedSubtype = "";
        g_SkinSelectedId = 0;
    }

    ImGui::Separator();

    if (g_SkinSelectedType == "Armor") {
        ImGui::Text("Weight Class");
        auto wcColor = [](const std::string& wc) -> ImVec4 {
            if (wc == "Heavy")  return ImVec4(0.65f, 0.65f, 0.70f, 1.0f);
            if (wc == "Medium") return ImVec4(0.65f, 0.50f, 0.30f, 1.0f);
            if (wc == "Light")  return ImVec4(0.55f, 0.45f, 0.70f, 1.0f);
            return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        };
        for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
            if (wc == "Clothing") continue;
            size_t count = Skinventory::SkinCache::GetCategorySkinCount("Armor", g_SkinSelectedSubtype, wc);
            std::string label = wc + " (" + std::to_string(count) + ")";
            drawBullet(wcColor(wc));
            if (ImGui::Selectable(label.c_str(), g_SkinSelectedWeightClass == wc)) {
                g_SkinSelectedWeightClass = wc;
                g_SkinSelectedId = 0;
            }
        }

        ImGui::Separator();
        ImGui::Text("Slot");
        for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(g_SkinSelectedWeightClass)) {
            size_t count = Skinventory::SkinCache::GetCategorySkinCount("Armor", slot, g_SkinSelectedWeightClass);
            std::string label = slot + " (" + std::to_string(count) + ")";
            drawBullet(ImVec4(0.45f, 0.65f, 0.85f, 1.0f));
            if (ImGui::Selectable(label.c_str(), g_SkinSelectedSubtype == slot)) {
                g_SkinSelectedSubtype = slot;
                g_SkinSelectedId = 0;
            }
        }
    } else if (g_SkinSelectedType == "Weapon") {
        ImGui::Text("Weapon Type");
        for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
            size_t count = Skinventory::SkinCache::GetCategorySkinCount("Weapon", wt, "");
            std::string label = wt + " (" + std::to_string(count) + ")";
            drawBullet(ImVec4(0.85f, 0.45f, 0.35f, 1.0f));
            if (ImGui::Selectable(label.c_str(), g_SkinSelectedSubtype == wt)) {
                g_SkinSelectedSubtype = wt;
                g_SkinSelectedId = 0;
            }
        }
    }

    ImGui::EndChild();
}

static void RenderSkinList() {
    ImGui::BeginChild("SkinList", ImVec2(300, 0), true);

    ImGui::PushItemWidth(-1);
    ImGui::InputTextWithHint("##skinfilter", "Filter skins...", g_SkinSearchFilter, sizeof(g_SkinSearchFilter));
    ImGui::PopItemWidth();

    if (Skinventory::OwnedSkins::HasData()) {
        ImGui::Checkbox("Owned", &g_SkinShowOwned);
        ImGui::SameLine();
        ImGui::Checkbox("Unowned", &g_SkinShowUnowned);
    }

    ImGui::Separator();

    std::vector<uint32_t> skins;
    if (g_SkinSelectedType == "Armor") {
        skins = Skinventory::SkinCache::GetSkinsByCategory("Armor", g_SkinSelectedSubtype, g_SkinSelectedWeightClass);
    } else if (g_SkinSelectedType == "Weapon") {
        skins = Skinventory::SkinCache::GetSkinsByCategory("Weapon", g_SkinSelectedSubtype, "");
    } else if (g_SkinSelectedType == "Back") {
        skins = Skinventory::SkinCache::GetSkinsByCategory("Back", "", "");
    }

    std::string filterLower(g_SkinSearchFilter);
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

    int ownedCount = 0;
    int totalCount = 0;

    bool showStatus = Skinventory::OwnedSkins::HasData();
    float statusHeight = showStatus ? (ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y) : 0;
    float scrollHeight = ImGui::GetContentRegionAvail().y - statusHeight;

    // Pre-filter skins into a display list for clipping
    struct SkinDisplayEntry { uint32_t id; bool owned; };
    std::vector<SkinDisplayEntry> displaySkins;
    displaySkins.reserve(skins.size());
    bool hasOwnerData = Skinventory::OwnedSkins::HasData();
    for (uint32_t skinId : skins) {
        auto skinOpt = Skinventory::SkinCache::GetSkin(skinId);
        if (!skinOpt || skinOpt->name.empty()) continue;

        totalCount++;
        bool owned = Skinventory::OwnedSkins::IsOwned(skinId);
        if (owned) ownedCount++;

        if (hasOwnerData) {
            if (owned && !g_SkinShowOwned) continue;
            if (!owned && !g_SkinShowUnowned) continue;
        }

        if (!filterLower.empty()) {
            std::string nameLower = skinOpt->name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos) continue;
        }

        displaySkins.push_back({skinId, owned});
    }

    ImGui::BeginChild("SkinListScroll", ImVec2(0, scrollHeight));
    ImGuiListClipper clipper;
    clipper.Begin((int)displaySkins.size());
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
            const auto& entry = displaySkins[row];
            auto skinOpt = Skinventory::SkinCache::GetSkin(entry.id);
            if (!skinOpt) continue;
            const auto& skin = *skinOpt;

            if (row % 2 == 1) {
                ImVec2 rowMin = ImGui::GetCursorScreenPos();
                ImVec2 rowMax(rowMin.x + ImGui::GetContentRegionAvail().x,
                              rowMin.y + ImGui::GetTextLineHeightWithSpacing());
                ImGui::GetWindowDrawList()->AddRectFilled(rowMin, rowMax,
                    ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.28f, 0.35f)));
            }

            bool selected = (g_SkinSelectedId == entry.id);
            if (hasOwnerData) {
                if (entry.owned) {
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "+");
                } else {
                    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), " -");
                }
                ImGui::SameLine(0, 4);
            }
            std::string label = skin.name + "##" + std::to_string(entry.id);
            ImGui::PushStyleColor(ImGuiCol_Text, GetSkinRarityColor(skin.rarity));
            if (ImGui::Selectable(label.c_str(), selected)) {
                g_SkinSelectedId = entry.id;
                Skinventory::WikiImage::RequestImage(entry.id, skin.name, skin.weight_class);
                Skinventory::Commerce::FetchPriceForSkin(entry.id);
            }
            ImGui::PopStyleColor();
            if (selected && g_SkinScrollToSkin) {
                ImGui::SetScrollHereY(0.5f);
                g_SkinScrollToSkin = false;
            }
        }
    }
    ImGui::EndChild();

    if (showStatus && totalCount > 0) {
        ImGui::Separator();
        float fraction = (float)ownedCount / (float)totalCount;
        char overlay[64];
        snprintf(overlay, sizeof(overlay), "%d / %d owned", ownedCount, totalCount);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.25f, 0.65f, 0.25f, 1.0f));
        ImGui::ProgressBar(fraction, ImVec2(-1, 0), overlay);
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

static void RenderSkinDetailPanel() {
    ImGui::BeginChild("SkinDetailPanel", ImVec2(0, 0), true);

    if (g_SkinSelectedId == 0) {
        ImGui::TextWrapped("Select a skin to view details.");
        ImGui::EndChild();
        return;
    }

    auto skinOpt = Skinventory::SkinCache::GetSkin(g_SkinSelectedId);
    if (!skinOpt) {
        ImGui::Text("Skin not found.");
        ImGui::EndChild();
        return;
    }
    const auto& skin = *skinOpt;

    auto sectionHeader = [](const char* label) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.75f, 0.9f, 1.0f), "%s", label);
        ImGui::Separator();
    };

    ImGui::PushStyleColor(ImGuiCol_Text, GetSkinRarityColor(skin.rarity));
    ImGui::TextWrapped("%s", skin.name.c_str());
    ImGui::PopStyleColor();

    ImGui::Text("%s%s%s",
        skin.type.c_str(),
        skin.subtype.empty() ? "" : (" / " + skin.subtype).c_str(),
        skin.weight_class.empty() ? "" : (" / " + skin.weight_class).c_str());
    ImGui::TextColored(GetSkinRarityColor(skin.rarity), "%s", skin.rarity.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(ID: %u)", skin.id);

    if (Skinventory::OwnedSkins::HasData()) {
        bool owned = Skinventory::OwnedSkins::IsOwned(g_SkinSelectedId);
        if (owned) {
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "OWNED");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "NOT OWNED");
        }
    }

    // Pricing (unowned only)
    bool isOwned = Skinventory::OwnedSkins::HasData() &&
                   Skinventory::OwnedSkins::IsOwned(g_SkinSelectedId);
    if (!isOwned) {
        int vendorPrice = Skinventory::Commerce::GetVendorPrice(g_SkinSelectedId);
        const auto* price = Skinventory::Commerce::GetPrice(g_SkinSelectedId);
        bool hasPrice = (vendorPrice > 0) ||
                        (price && price->tradeable && (price->sell_price > 0 || price->buy_price > 0)) ||
                        (!price && vendorPrice == 0);

        if (hasPrice) {
            sectionHeader("Pricing");

            if (vendorPrice > 0) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Vendor:");
                ImGui::SameLine();
                RenderCoins(vendorPrice);
            }

            if (price) {
                if (price->tradeable) {
                    if (price->sell_price > 0) {
                        ImGui::Text("TP Buy:");
                        ImGui::SameLine();
                        RenderCoins(price->sell_price);
                    }
                    if (price->buy_price > 0) {
                        ImGui::Text("TP Sell:");
                        ImGui::SameLine();
                        RenderCoins(price->buy_price);
                    }
                }
            } else if (vendorPrice == 0) {
                if (Skinventory::Commerce::IsItemMapReady()) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Price: click to load");
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Item map loading...");
                }
            }
        }
    }

    // Acquisition
    {
        auto wikiData = Skinventory::WikiImage::GetWikiData(g_SkinSelectedId);
        bool hasAny = !wikiData.acquisition.empty() || !wikiData.collection.empty() ||
                      !wikiData.set_name.empty() || !wikiData.vendor_name.empty();
        if (hasAny) {
            sectionHeader("Acquisition");
            if (!wikiData.acquisition.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Source:");
                ImGui::SameLine();
                ImGui::TextWrapped("%s", wikiData.acquisition.c_str());
            }
            if (!wikiData.collection.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Collection:");
                ImGui::SameLine();
                ImGui::TextWrapped("%s", wikiData.collection.c_str());
            }
            if (!wikiData.set_name.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Set:");
                ImGui::SameLine();
                ImGui::TextWrapped("%s", wikiData.set_name.c_str());
            }
            if (!wikiData.vendor_name.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Vendor:");
                ImGui::SameLine();
                if (!wikiData.vendor_location.empty()) {
                    ImGui::TextWrapped("%s (%s)", wikiData.vendor_name.c_str(),
                                       wikiData.vendor_location.c_str());
                } else {
                    ImGui::TextWrapped("%s", wikiData.vendor_name.c_str());
                }
            }
            if (!wikiData.vendor_cost.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Cost:");
                ImGui::SameLine();
                std::string costStr = wikiData.vendor_cost;
                std::string keysStr = wikiData.vendor_cost_keys;
                std::vector<std::string> iconKeys;
                {
                    std::istringstream kss(keysStr);
                    std::string kk;
                    while (std::getline(kss, kk, ',')) {
                        if (!kk.empty()) iconKeys.push_back(kk);
                    }
                }
                std::vector<std::string> costParts;
                {
                    size_t p = 0;
                    while (true) {
                        size_t sep = costStr.find(" + ", p);
                        if (sep == std::string::npos) {
                            costParts.push_back(costStr.substr(p));
                            break;
                        }
                        costParts.push_back(costStr.substr(p, sep - p));
                        p = sep + 3;
                    }
                }
                for (size_t ci = 0; ci < costParts.size(); ci++) {
                    if (ci > 0) {
                        ImGui::SameLine(0, 2);
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "+");
                        ImGui::SameLine(0, 2);
                    }
                    if (ci < iconKeys.size()) {
                        Texture_t* cIcon = Skinventory::WikiImage::GetCurrencyIcon(iconKeys[ci]);
                        if (cIcon && cIcon->Resource) {
                            float iconH = ImGui::GetTextLineHeight();
                            float iconW = iconH * ((float)cIcon->Width / (float)cIcon->Height);
                            ImGui::Image(cIcon->Resource, ImVec2(iconW, iconH));
                            ImGui::SameLine(0, 3);
                        }
                    }
                    ImGui::TextColored(ImVec4(0.9f, 0.7f, 1.0f, 1.0f), "%s", costParts[ci].c_str());
                }
            }
            if (!wikiData.vendor_waypoint.empty()) {
                static float s_copiedTimer = 0.0f;
                static uint32_t s_copiedSkinId = 0;

                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Waypoint:");
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
                if (ImGui::SmallButton(wikiData.vendor_waypoint.c_str())) {
                    if (OpenClipboard(NULL)) {
                        EmptyClipboard();
                        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,
                            wikiData.vendor_waypoint.size() + 1);
                        if (hMem) {
                            char* pMem = (char*)GlobalLock(hMem);
                            memcpy(pMem, wikiData.vendor_waypoint.c_str(),
                                   wikiData.vendor_waypoint.size() + 1);
                            GlobalUnlock(hMem);
                            SetClipboardData(CF_TEXT, hMem);
                        }
                        CloseClipboard();
                    }
                    s_copiedTimer = 2.0f;
                    s_copiedSkinId = g_SkinSelectedId;
                }
                ImGui::PopStyleColor();

                if (s_copiedTimer > 0.0f && s_copiedSkinId == g_SkinSelectedId) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, s_copiedTimer / 2.0f),
                                       "Copied!");
                    s_copiedTimer -= ImGui::GetIO().DeltaTime;
                }
            }
        }
    }

    // Preview
    sectionHeader("Preview");

    Texture_t* wikiTex = Skinventory::WikiImage::GetImage(g_SkinSelectedId);
    if (wikiTex && wikiTex->Resource) {
        float panelWidth = ImGui::GetContentRegionAvail().x;
        float maxHeight = 400.0f;
        float imgW = (float)wikiTex->Width;
        float imgH = (float)wikiTex->Height;

        float scale = panelWidth / imgW;
        if (imgH * scale > maxHeight) {
            scale = maxHeight / imgH;
        }

        float displayW = imgW * scale;
        float displayH = imgH * scale;

        ImGui::Image(wikiTex->Resource, ImVec2(displayW, displayH));
    } else if (Skinventory::WikiImage::IsLoading(g_SkinSelectedId)) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Loading wiki image...");
    }

    // Action buttons
    ImGui::Separator();
    if (ImGui::Button("Open Wiki Page")) {
        std::string wikiUrl = "https://wiki.guildwars2.com/wiki/" + skin.name;
        std::replace(wikiUrl.begin(), wikiUrl.end(), ' ', '_');
        ShellExecuteA(NULL, "open", wikiUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    ImGui::EndChild();
}

static void RenderSkinShoppingList() {
    if (!Skinventory::OwnedSkins::HasData()) {
        ImGui::TextWrapped("Hoard & Seek data not yet loaded. Ensure Hoard & Seek is installed and has fetched account data.");
        return;
    }

    if (!Skinventory::Commerce::IsItemMapReady()) {
        std::string fetchMsg = Skinventory::Commerce::GetFetchStatus();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", fetchMsg.c_str());
        ImGui::TextWrapped("Building the skin-to-item map from the GW2 API. This is a one-time operation and will be cached for future sessions.");
        return;
    }

    static bool s_needsFetch = true;
    if (ImGui::Button("Refresh Prices") && !Skinventory::Commerce::IsFetching()) {
        g_SkinShopListDirty = true;
        s_needsFetch = true;
    }
    ImGui::SameLine();
    ImGui::Text("Category:");
    ImGui::SameLine();
    static int shopTypeFilter = 0;
    int prevFilter = shopTypeFilter;
    ImGui::RadioButton("All##shop", &shopTypeFilter, 0); ImGui::SameLine();
    ImGui::RadioButton("Armor##shop", &shopTypeFilter, 1); ImGui::SameLine();
    ImGui::RadioButton("Weapons##shop", &shopTypeFilter, 2);
    if (shopTypeFilter != prevFilter) {
        g_SkinShopListDirty = true;
    }

    ImGui::Text("Source:");
    ImGui::SameLine();
    static int shopSourceFilter = 0;
    int prevSource = shopSourceFilter;
    ImGui::RadioButton("All##src", &shopSourceFilter, 0); ImGui::SameLine();
    ImGui::RadioButton("TP##src", &shopSourceFilter, 1); ImGui::SameLine();
    ImGui::RadioButton("Vendor##src", &shopSourceFilter, 2);
    if (shopSourceFilter != prevSource) {
        g_SkinShopListDirty = true;
    }

    if (Skinventory::Commerce::IsFetching()) {
        std::string fetchMsg = Skinventory::Commerce::GetFetchStatus();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", fetchMsg.c_str());
    }

    ImGui::Separator();

    static bool s_wasFetching = false;
    bool isFetching = Skinventory::Commerce::IsFetching();
    if (s_wasFetching && !isFetching) {
        g_SkinShopListDirty = true;
        s_needsFetch = false;
    }
    s_wasFetching = isFetching;

    if (g_SkinShopListDirty && !isFetching) {
        g_SkinShopList.clear();

        std::vector<uint32_t> to_price;

        auto collectSkins = [&](const std::string& type, const std::string& subtype,
                                 const std::string& weight) {
            auto skins = Skinventory::SkinCache::GetSkinsByCategory(type, subtype, weight);
            for (uint32_t id : skins) {
                if (!Skinventory::OwnedSkins::IsOwned(id)) {
                    to_price.push_back(id);
                }
            }
        };

        if (shopTypeFilter == 0 || shopTypeFilter == 1) {
            for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
                for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(wc)) {
                    collectSkins("Armor", slot, wc);
                }
            }
        }
        if (shopTypeFilter == 0 || shopTypeFilter == 2) {
            for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
                collectSkins("Weapon", wt, "");
            }
        }

        if (s_needsFetch) {
            Skinventory::Commerce::FetchPricesForSkins(to_price);
        }

        for (uint32_t id : to_price) {
            int vendorPrice = Skinventory::Commerce::GetVendorPrice(id);
            const auto* p = Skinventory::Commerce::GetPrice(id);
            int tpPrice = (p && p->tradeable && p->sell_price > 0) ? p->sell_price : 0;

            bool useVendor = false;
            bool useTP = false;

            if (vendorPrice > 0 && tpPrice > 0) {
                if (vendorPrice <= tpPrice) useVendor = true;
                else useTP = true;
            } else if (vendorPrice > 0) {
                useVendor = true;
            } else if (tpPrice > 0) {
                useTP = true;
            }

            if (useVendor && shopSourceFilter == 1) { useVendor = false; useTP = (tpPrice > 0); }
            if (useTP && shopSourceFilter == 2) { useTP = false; useVendor = (vendorPrice > 0); }

            if (useVendor) {
                g_SkinShopList.push_back({id, vendorPrice, 1});
            } else if (useTP) {
                g_SkinShopList.push_back({id, tpPrice, 0});
            }
        }

        std::sort(g_SkinShopList.begin(), g_SkinShopList.end(),
            [](const auto& a, const auto& b) { return a.price < b.price; });

        g_SkinShopListDirty = false;
    }

    int tpCount = 0, vendorCount = 0;
    for (const auto& e : g_SkinShopList) {
        if (e.source == 0) tpCount++;
        else vendorCount++;
    }

    ImGui::Text("Cheapest unowned skins: %d TP, %d Vendor (%zu total)",
                tpCount, vendorCount, g_SkinShopList.size());
    ImGui::Separator();

    float reserveHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f;
    ImVec2 tableSize(0.0f, -reserveHeight);
    if (ImGui::BeginTable("ShoppingList", 5,
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Sortable,
        tableSize)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80, 1);
        ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed, 55, 2);
        ImGui::TableSetupColumn("Rarity", ImGuiTableColumnFlags_WidthFixed, 65, 3);
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 120, 4);
        ImGui::TableHeadersRow();

        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
            if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];
                bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending);
                int col = spec.ColumnUserID;

                std::sort(g_SkinShopList.begin(), g_SkinShopList.end(),
                    [col, ascending](const auto& a, const auto& b) {
                        if (col == 4) {
                            return ascending ? a.price < b.price : a.price > b.price;
                        }
                        if (col == 2) {
                            return ascending ? a.source < b.source : a.source > b.source;
                        }
                        auto sa = Skinventory::SkinCache::GetSkin(a.skinId);
                        auto sb = Skinventory::SkinCache::GetSkin(b.skinId);
                        if (!sa || !sb) return false;

                        int cmp = 0;
                        if (col == 0) {
                            cmp = sa->name.compare(sb->name);
                        } else if (col == 1) {
                            cmp = sa->subtype.compare(sb->subtype);
                        } else if (col == 3) {
                            cmp = sa->rarity.compare(sb->rarity);
                        }
                        return ascending ? cmp < 0 : cmp > 0;
                    });
                sortSpecs->SpecsDirty = false;
            }
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)g_SkinShopList.size());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                const auto& entry = g_SkinShopList[row];
                auto skinOpt = Skinventory::SkinCache::GetSkin(entry.skinId);
                if (!skinOpt) continue;
                const auto& skin = *skinOpt;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                std::string selectLabel = skin.name + "##shop" + std::to_string(entry.skinId);
                ImGui::PushStyleColor(ImGuiCol_Text, GetSkinRarityColor(skin.rarity));
                if (ImGui::Selectable(selectLabel.c_str(), g_SkinSelectedId == entry.skinId,
                    ImGuiSelectableFlags_SpanAllColumns)) {
                    g_SkinSelectedId = entry.skinId;
                    Skinventory::WikiImage::RequestImage(entry.skinId, skin.name, skin.weight_class);
                    Skinventory::Commerce::FetchPriceForSkin(entry.skinId);
                    g_SkinSelectedType = skin.type;
                    if (skin.type == "Armor") {
                        g_SkinSelectedWeightClass = skin.weight_class;
                    }
                    g_SkinSelectedSubtype = skin.subtype;
                    g_SkinSwitchToBrowser = true;
                    g_SkinScrollToSkin = true;
                }
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();
                ImGui::Text("%s", skin.subtype.c_str());

                ImGui::TableNextColumn();
                if (entry.source == 1) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Vendor");
                } else {
                    ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.3f, 1.0f), "TP");
                }

                ImGui::TableNextColumn();
                ImGui::TextColored(GetSkinRarityColor(skin.rarity), "%s", skin.rarity.c_str());

                ImGui::TableNextColumn();
                RenderCoins(entry.price);
            }
        }

        ImGui::EndTable();
    }

    if (!g_SkinShopList.empty()) {
        int tpTotal = 0, vendorTotal = 0;
        for (const auto& e : g_SkinShopList) {
            if (e.source == 0) tpTotal += e.price;
            else vendorTotal += e.price;
        }
        ImGui::Separator();
        ImGui::Text("TP total:");
        ImGui::SameLine();
        RenderCoins(tpTotal);
        if (vendorTotal > 0) {
            ImGui::SameLine();
            ImGui::Text("  Vendor total:");
            ImGui::SameLine();
            RenderCoins(vendorTotal);
        }
    }
}

// Helper: strip fractal achievement name to short form
static std::string ShortenFractalName(const std::string& name) {
    // "Daily Recommended Fractal—Swampland" → "Swampland"
    auto pos = name.find("Fractal");
    if (pos != std::string::npos && pos + 7 < name.size()) {
        size_t start = pos + 7;
        while (start < name.size() && (name[start] == ' ' || (unsigned char)name[start] == 0xe2)) {
            if ((unsigned char)name[start] == 0xe2 && start + 2 < name.size())
                start += 3;
            else
                start++;
        }
        return name.substr(start);
    }
    // "Daily Tier 4 Swampland" → "Swampland"
    auto pos2 = name.find("Tier");
    if (pos2 != std::string::npos) {
        size_t afterTier = name.find(' ', pos2 + 5);
        if (afterTier != std::string::npos && afterTier + 1 < name.size())
            return name.substr(afterTier + 1);
    }
    return name;
}

// Helper: render a done/not-done/unknown indicator
static void RenderClearStatus(bool fetched, bool done, const char* label) {
    if (!fetched)
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "[?] %s", label);
    else if (done)
        ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "[x] %s", label);
    else
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "[ ] %s", label);
}

static void RenderClears() {
    auto now = std::chrono::system_clock::now();

    // Compute reset times
    auto dailyReset = CalcLastDailyReset(now);
    auto weeklyReset = CalcLastWeeklyReset(now);

    // Detect resets — clear stale data and auto re-fetch achievement lists
    bool resetTriggered = false;
    if (g_LastDailyReset != std::chrono::system_clock::time_point{} && dailyReset > g_LastDailyReset) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_DailyFractals.clear();
        g_DailyBounties.clear();
        g_ClearsFetched = false;
        resetTriggered = true;
    }
    if (g_LastWeeklyReset != std::chrono::system_clock::time_point{} && weeklyReset > g_LastWeeklyReset) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_WeeklyWings.clear();
        g_WeeklyStrikes = ClearEntry{};
        g_ClearsFetched = false;
        resetTriggered = true;
    }
    g_LastDailyReset = dailyReset;
    g_LastWeeklyReset = weeklyReset;

    // Auto re-fetch full data at reset boundaries
    if (resetTriggered && !g_ClearsFetching) {
        FetchClears();
    }

    // Auto re-query completion every 10 minutes (if we have data and not currently fetching)
    auto steadyNow = std::chrono::steady_clock::now();
    if (g_ClearsFetched && !g_ClearsFetching &&
        g_LastClearsCompletionQuery != std::chrono::steady_clock::time_point{} &&
        (steadyNow - g_LastClearsCompletionQuery) >= std::chrono::minutes(10)) {
        g_LastClearsCompletionQuery = steadyNow;
        SendClearsAchQuery();
    }

    // Refresh button + status
    bool fetching = g_ClearsFetching;
    if (fetching) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    if (ImGui::SmallButton("Refresh##clears") && !fetching) {
        FetchClears();
    }
    if (fetching) ImGui::PopStyleVar();
    ImGui::SameLine();
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        if (fetching) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", g_ClearsStatusMsg.c_str());
        } else if (!g_ClearsFetched && g_DailyFractals.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Click Refresh to load data");
        } else if (!g_ClearsStatusMsg.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", g_ClearsStatusMsg.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Data loaded");
        }
    }

    ImGui::Separator();

    std::lock_guard<std::mutex> lock(g_ClearsMutex);

    std::string dailyResetStr = FormatTimeUntilReset(dailyReset, std::chrono::hours(24));
    std::string weeklyResetStr = FormatTimeUntilReset(weeklyReset, std::chrono::hours(24 * 7));

    // ---- Daily Fractals ----
    ImGui::TextColored(ImVec4(0.3f, 0.7f, 0.9f, 1.0f), "Daily Fractals");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(resets in %s)", dailyResetStr.c_str());

    if (g_DailyFractals.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        const char* tierOrder[] = {"T4", "T3", "T2", "T1", "Rec"};
        for (const char* tier : tierOrder) {
            std::vector<const ClearEntry*> tierEntries;
            for (const auto& e : g_DailyFractals) {
                if (e.tier == tier) tierEntries.push_back(&e);
            }
            if (tierEntries.empty()) continue;

            // Sort by shortened name so order is consistent across tiers
            std::sort(tierEntries.begin(), tierEntries.end(),
                [](const ClearEntry* a, const ClearEntry* b) {
                    return ShortenFractalName(a->name) < ShortenFractalName(b->name);
                });

            bool allDone = g_ClearsFetched;
            for (const auto* e : tierEntries) {
                if (!e->done) { allDone = false; break; }
            }

            if (allDone)
                ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "  %-4s", tier);
            else
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "  %-4s", tier);

            for (const auto* e : tierEntries) {
                ImGui::SameLine();
                std::string shortName = ShortenFractalName(e->name);
                RenderClearStatus(g_ClearsFetched, e->done, shortName.c_str());
            }
        }
        // Uncategorized
        for (const auto& e : g_DailyFractals) {
            if (!e.tier.empty()) continue;
            ImGui::Text("  ");
            ImGui::SameLine();
            RenderClearStatus(g_ClearsFetched, e.done, e.name.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Daily Raid Bounties ----
    ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.3f, 1.0f), "Daily Raid Bounties");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(resets in %s)", dailyResetStr.c_str());

    if (g_DailyBounties.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        for (const auto& e : g_DailyBounties) {
            // Strip "Raid Bounty: " prefix
            std::string display = e.name;
            if (display.find("Raid Bounty: ") == 0)
                display = display.substr(13);
            ImGui::Text("  ");
            ImGui::SameLine();
            RenderClearStatus(g_ClearsFetched, e.done, display.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Weekly Strikes ----
    ImGui::TextColored(ImVec4(0.7f, 0.4f, 0.9f, 1.0f), "Weekly Strikes");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(resets in %s)", weeklyResetStr.c_str());

    if (g_WeeklyStrikes.id == 0) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        for (size_t i = 0; i < g_WeeklyStrikes.bitNames.size(); i++) {
            bool bitDone = (i < g_WeeklyStrikes.bitDone.size()) ? g_WeeklyStrikes.bitDone[i] : false;
            ImGui::Text("  ");
            ImGui::SameLine();
            RenderClearStatus(g_ClearsFetched, bitDone, g_WeeklyStrikes.bitNames[i].c_str());
        }
        if (g_ClearsFetched && g_WeeklyStrikes.max > 0) {
            int doneCount = 0;
            for (bool b : g_WeeklyStrikes.bitDone) { if (b) doneCount++; }
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  %d / %d completed",
                doneCount, (int)g_WeeklyStrikes.bitNames.size());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Weekly Raids ----
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "Weekly Raids");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(resets in %s)", weeklyResetStr.c_str());

    if (g_WeeklyWings.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        if (ImGui::BeginTable("RaidWingsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Wing", ImGuiTableColumnFlags_WidthFixed, 220);
            ImGui::TableSetupColumn("Encounters", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto& wing : g_WeeklyWings) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // Strip "Weekly " prefix
                std::string wingName = wing.name;
                if (wingName.find("Weekly ") == 0)
                    wingName = wingName.substr(7);

                bool allDone = g_ClearsFetched && wing.done;
                if (allDone)
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "%s", wingName.c_str());
                else
                    ImGui::Text("%s", wingName.c_str());

                ImGui::TableNextColumn();
                for (size_t i = 0; i < wing.bitNames.size(); i++) {
                    bool bitDone = (i < wing.bitDone.size()) ? wing.bitDone[i] : false;
                    if (i > 0) ImGui::SameLine();
                    RenderClearStatus(g_ClearsFetched, bitDone, wing.bitNames[i].c_str());
                    if (i + 1 < wing.bitNames.size()) {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "|");
                    }
                }
            }
            ImGui::EndTable();
        }
    }
}

static void RenderSkinventory() {
    auto cacheStatus = Skinventory::SkinCache::GetStatus();

    if (cacheStatus == Skinventory::CacheStatus::Loading ||
        cacheStatus == Skinventory::CacheStatus::Empty) {
        std::string msg = Skinventory::SkinCache::GetStatusMessage();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", msg.c_str());
        return;
    }

    if (cacheStatus == Skinventory::CacheStatus::Error) {
        std::string msg = Skinventory::SkinCache::GetStatusMessage();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", msg.c_str());
        return;
    }

    // Status bar + manual refresh
    {
        static auto s_lastRefreshTime = std::chrono::steady_clock::time_point{};
        static bool s_refreshPending = false;

        // Consume H&S data-updated flag (e.g. user refreshed in H&S)
        if (Skinventory::OwnedSkins::ConsumeDataUpdatedFlag()) {
            g_SkinRefreshOwned = true;
        }

        bool isQuerying = Skinventory::OwnedSkins::IsQuerying();
        bool canRefresh = Skinventory::OwnedSkins::IsHoardAndSeekAvailable() && !isQuerying;

        if (!canRefresh) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        if (ImGui::SmallButton("Refresh Owned") && canRefresh) {
            g_SkinRefreshOwned = true;
        }
        if (!canRefresh) ImGui::PopStyleVar();

        // Kick off batch query when requested
        if (g_SkinRefreshOwned && canRefresh) {
            g_SkinRefreshOwned = false;
            s_refreshPending = true;

            std::vector<uint32_t> allIds;
            auto collectAll = [&](const std::string& type, const std::string& subtype,
                                   const std::string& weight) {
                auto ids = Skinventory::SkinCache::GetSkinsByCategory(type, subtype, weight);
                allIds.insert(allIds.end(), ids.begin(), ids.end());
            };
            for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
                for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(wc)) {
                    collectAll("Armor", slot, wc);
                }
            }
            for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
                collectAll("Weapon", wt, "");
            }
            collectAll("Back", "", "");
            if (!allIds.empty()) {
                Skinventory::OwnedSkins::RequestOwnedSkins(allIds);
            }
        }

        // Detect when query finishes
        if (s_refreshPending && !isQuerying) {
            s_refreshPending = false;
            s_lastRefreshTime = std::chrono::steady_clock::now();
        }

        // Status text
        ImGui::SameLine();
        if (isQuerying) {
            std::string hsMsg = Skinventory::OwnedSkins::GetStatusMessage();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", hsMsg.c_str());
        } else if (Skinventory::OwnedSkins::HasData()) {
            size_t owned = Skinventory::OwnedSkins::GetOwnedCount();
            if (s_lastRefreshTime != std::chrono::steady_clock::time_point{}) {
                auto elapsed = std::chrono::steady_clock::now() - s_lastRefreshTime;
                int secs = (int)std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
                if (secs < 60)
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%zu owned  |  Refreshed %ds ago", owned, secs);
                else
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%zu owned  |  Refreshed %dm ago", owned, secs / 60);
            } else {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%zu owned (cached)", owned);
            }
        } else if (!Skinventory::OwnedSkins::IsHoardAndSeekAvailable()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Waiting for Hoard & Seek...");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Click Refresh Owned to query skins");
        }

        if (cacheStatus == Skinventory::CacheStatus::Updating) {
            ImGui::SameLine();
            std::string updateMsg = Skinventory::SkinCache::GetStatusMessage();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "| %s", updateMsg.c_str());
        }
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("##skin_tabs")) {
        ImGuiTabItemFlags browserFlags = 0;
        if (g_SkinSwitchToBrowser) {
            browserFlags = ImGuiTabItemFlags_SetSelected;
            g_SkinSwitchToBrowser = false;
        }
        if (ImGui::BeginTabItem("Browser", nullptr, browserFlags)) {
            g_SkinActiveTab = 0;
            RenderSkinCategoryNav();
            ImGui::SameLine();
            RenderSkinList();
            ImGui::SameLine();
            RenderSkinDetailPanel();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Shopping List")) {
            g_SkinActiveTab = 1;
            RenderSkinShoppingList();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

// --- Main Render ---

void AddonRender() {
    // Process icon download queue every frame
    AlterEgo::IconManager::Tick();
    Skinventory::WikiImage::Tick();
    Skinventory::OwnedSkins::Tick();

    // Render gear customize dialog (separate window, always checked)
    RenderGearCustomizeDialog();

    if (!g_WindowVisible) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX));
    if (!ImGui::Begin("Alter Ego", &g_WindowVisible, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // H&S status + Refresh button
    auto hoardStatus = AlterEgo::GW2API::GetHoardStatus();
    auto fetchStatus = AlterEgo::GW2API::GetFetchStatus();
    bool scanning = (fetchStatus == AlterEgo::FetchStatus::InProgress);
    bool hoardReady = (hoardStatus == AlterEgo::HoardStatus::Available ||
                       hoardStatus == AlterEgo::HoardStatus::Ready);
    bool disabled = scanning || !hoardReady;

    if (disabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    if (ImGui::Button("Refresh Characters") && !disabled) {
        // Fetch just the character name list, then show selection popup
        g_RefreshListFetching = true;
        AlterEgo::GW2API::RequestCharacterList();
    }
    if (disabled) ImGui::PopStyleVar();

    // When the list-only fetch completes, populate the popup
    if (g_RefreshListFetching &&
        AlterEgo::GW2API::GetFetchStatus() != AlterEgo::FetchStatus::InProgress) {
        g_RefreshListFetching = false;
        const auto& names = AlterEgo::GW2API::GetPendingCharNames();
        if (!names.empty()) {
            g_RefreshNames = names;
            // Pre-select: currently viewed character checked, rest unchecked
            g_RefreshSelection.assign(names.size(), false);
            const auto& chars = AlterEgo::GW2API::GetCharacters();
            std::string currentName;
            if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                if (ri >= 0 && ri < (int)chars.size()) currentName = chars[ri].name;
            }
            for (size_t i = 0; i < names.size(); i++) {
                if (names[i] == currentName)
                    g_RefreshSelection[i] = true;
            }
            g_RefreshPopupOpen = true;
            ImGui::OpenPopup("Select Characters to Refresh");
        }
    }

    ImGui::SameLine();

    // H&S connection indicator
    if (hoardStatus == AlterEgo::HoardStatus::Unknown) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Checking for H&S...");
        // Retry ping periodically
        static auto lastPing = std::chrono::steady_clock::time_point{};
        auto now = std::chrono::steady_clock::now();
        if (now - lastPing > std::chrono::seconds(5)) {
            lastPing = now;
            AlterEgo::GW2API::PingHoard();
        }
    } else if (hoardStatus == AlterEgo::HoardStatus::Unavailable) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "H&S not detected. Install Hoard & Seek.");
    } else if (hoardStatus == AlterEgo::HoardStatus::PermPending) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Approve Alter Ego in H&S permission popup.");
        // Retry request every 3 seconds until user accepts/denies.
        static auto lastPermRetry = std::chrono::steady_clock::time_point{};
        auto permNow = std::chrono::steady_clock::now();
        if (permNow - lastPermRetry > std::chrono::seconds(3)) {
            lastPermRetry = permNow;
            AlterEgo::GW2API::RequestCharacterList();
        }
    } else if (hoardStatus == AlterEgo::HoardStatus::PermDenied) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "H&S permission denied. Enable in H&S settings.");
    } else if (fetchStatus == AlterEgo::FetchStatus::InProgress) {
        g_FetchDoneTime = std::chrono::steady_clock::time_point{}; // reset fade timer while in progress
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s",
            AlterEgo::GW2API::GetFetchStatusMessage().c_str());
    } else if (fetchStatus == AlterEgo::FetchStatus::Error ||
               fetchStatus == AlterEgo::FetchStatus::Success) {
        // Record completion time on first frame after finishing
        if (g_FetchDoneTime == std::chrono::steady_clock::time_point{})
            g_FetchDoneTime = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - g_FetchDoneTime).count();
        if (elapsed < 5.0f) {
            float alpha = (elapsed < 4.0f) ? 1.0f : (5.0f - elapsed); // fade during last second
            ImVec4 col = (fetchStatus == AlterEgo::FetchStatus::Error)
                ? ImVec4(1.0f, 0.3f, 0.3f, alpha)
                : ImVec4(0.35f, 0.82f, 0.35f, alpha);
            ImGui::TextColored(col, "%s",
                AlterEgo::GW2API::GetFetchStatusMessage().c_str());
        }
        // After 5s, fall through to show "Last updated" below
        if (elapsed >= 5.0f && !AlterEgo::GW2API::HasCharacterData()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "H&S connected. Click Refresh to load characters.");
        } else if (elapsed >= 5.0f) {
            time_t last = AlterEgo::GW2API::GetLastUpdated();
            if (last > 0) {
                time_t now_t = std::time(nullptr);
                int el = (int)difftime(now_t, last);
                std::string ago;
                if (el < 60) ago = "just now";
                else if (el < 3600) ago = std::to_string(el / 60) + "m ago";
                else if (el < 86400) ago = std::to_string(el / 3600) + "h ago";
                else ago = std::to_string(el / 86400) + "d ago";
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Last updated %s", ago.c_str());
            }
        }
    } else if (!AlterEgo::GW2API::HasCharacterData()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
            "H&S connected. Click Refresh to load characters.");
    } else {
        time_t last = AlterEgo::GW2API::GetLastUpdated();
        if (last > 0) {
            time_t now_t = std::time(nullptr);
            int elapsed = (int)difftime(now_t, last);
            std::string ago;
            if (elapsed < 60) ago = "just now";
            else if (elapsed < 3600) ago = std::to_string(elapsed / 60) + "m ago";
            else if (elapsed < 86400) ago = std::to_string(elapsed / 3600) + "h ago";
            else ago = std::to_string(elapsed / 86400) + "d ago";
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Last updated %s", ago.c_str());
        }
    }

    // Character refresh selection popup
    if (ImGui::BeginPopup("Select Characters to Refresh")) {
        ImGui::Text("Select characters to refresh:");
        ImGui::Spacing();

        // All / None / Lv80 buttons
        if (ImGui::SmallButton("All")) {
            for (size_t i = 0; i < g_RefreshSelection.size(); i++)
                g_RefreshSelection[i] = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("None")) {
            for (size_t i = 0; i < g_RefreshSelection.size(); i++)
                g_RefreshSelection[i] = false;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Lv80")) {
            const auto& cachedChars = AlterEgo::GW2API::GetCharacters();
            for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                g_RefreshSelection[i] = false;
                for (const auto& cc : cachedChars) {
                    if (cc.name == g_RefreshNames[i] && cc.level == 80) {
                        g_RefreshSelection[i] = true;
                        break;
                    }
                }
            }
        }

        ImGui::Separator();

        // Scrollable checkbox list
        ImGui::BeginChild("##charCheckList", ImVec2(280, 300), true);
        const auto& cachedChars = AlterEgo::GW2API::GetCharacters();
        for (size_t i = 0; i < g_RefreshNames.size(); i++) {
            ImGui::PushID((int)i);
            bool checked = g_RefreshSelection[i];

            // Look up cached data for level/profession color
            const AlterEgo::Character* cached = nullptr;
            for (const auto& cc : cachedChars) {
                if (cc.name == g_RefreshNames[i]) { cached = &cc; break; }
            }

            if (ImGui::Checkbox("##cb", &checked))
                g_RefreshSelection[i] = checked;
            ImGui::SameLine();
            if (cached) {
                ImGui::TextColored(GetProfessionColor(cached->profession), "%s", g_RefreshNames[i].c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Lv%d", cached->level);
            } else {
                ImGui::Text("%s", g_RefreshNames[i].c_str());
            }
            ImGui::PopID();
        }
        ImGui::EndChild();

        // Count selected
        int selCount = 0;
        for (bool b : g_RefreshSelection) { if (b) selCount++; }

        ImGui::Spacing();
        char btnLabel[64];
        snprintf(btnLabel, sizeof(btnLabel), "Refresh Selected (%d)", selCount);
        bool canRefresh = (selCount > 0);
        if (!canRefresh) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        if (ImGui::Button(btnLabel) && canRefresh) {
            // Build the selected name list, with currently viewed character first
            std::vector<std::string> selected;
            const auto& chars = AlterEgo::GW2API::GetCharacters();
            std::string currentName;
            if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                if (ri >= 0 && ri < (int)chars.size()) currentName = chars[ri].name;
            }

            // Add current character first if selected
            for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                if (g_RefreshSelection[i] && g_RefreshNames[i] == currentName) {
                    selected.push_back(g_RefreshNames[i]);
                    break;
                }
            }
            // Add the rest
            for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                if (g_RefreshSelection[i] && g_RefreshNames[i] != currentName)
                    selected.push_back(g_RefreshNames[i]);
            }

            AlterEgo::GW2API::RequestCharacterRefreshSelected(selected);
            g_DetailsFetched = false;
            g_RefreshPopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        if (!canRefresh) ImGui::PopStyleVar();
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            g_RefreshPopupOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::Separator();

    // Top-level tab bar: Characters | Build Library
    if (ImGui::BeginTabBar("##main_tabs")) {
        if (ImGui::BeginTabItem("Characters")) {
            g_MainTab = 0;

            // Re-trigger detail fetch when character data finishes loading
            {
                static AlterEgo::FetchStatus lastFetchStatus = AlterEgo::FetchStatus::Idle;
                if (fetchStatus == AlterEgo::FetchStatus::Success && lastFetchStatus != AlterEgo::FetchStatus::Success) {
                    g_DetailsFetched = false;
                }
                lastFetchStatus = fetchStatus;
            }

            const auto& characters = AlterEgo::GW2API::GetCharacters();
            if (!characters.empty()) {
                // Rebuild display order when character count changes
                if (characters.size() != g_LastCharCount) {
                    RebuildCharDisplayOrder();
                }

                // Auto-select first character if none selected
                if (g_SelectedCharIdx < 0 || g_SelectedCharIdx >= (int)g_CharDisplayOrder.size()) {
                    g_SelectedCharIdx = 0;
                    g_DetailsFetched = false;
                    g_SelectedBuildTab = -1;
                }

                // Character list (left) + Detail panel (right)
                float listWidth = 200.0f;

                ImGui::BeginChild("CharList", ImVec2(listWidth, 0), true);

                // Sort mode selector + direction toggle (inside the list child)
                {
                    float arrowBtnWidth = (g_CharSortMode != Sort_Custom) ? 24.0f : 0.0f;
                    float avail = ImGui::GetContentRegionAvail().x;
                    float comboWidth = avail - arrowBtnWidth - (arrowBtnWidth > 0 ? 4.0f : 0.0f);
                    ImGui::SetNextItemWidth(comboWidth);
                    const char* sortLabels[] = { "Custom", "Name", "Class", "Level", "Age", "Birthday" };
                    if (ImGui::Combo("##sort", &g_CharSortMode, sortLabels, IM_ARRAYSIZE(sortLabels))) {
                        g_CharSortAscending = true; // reset direction on mode change
                        std::string selName;
                        if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                            int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                            if (ri >= 0 && ri < (int)characters.size()) selName = characters[ri].name;
                        }
                        RebuildCharDisplayOrder();
                        g_SelectedCharIdx = 0;
                        for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                            if (characters[g_CharDisplayOrder[di]].name == selName) {
                                g_SelectedCharIdx = di;
                                break;
                            }
                        }
                        SaveCharSortConfig();
                    }
                    if (g_CharSortMode != Sort_Custom) {
                        ImGui::SameLine();
                        if (ImGui::Button("##sortdir", ImVec2(arrowBtnWidth, 0))) {
                            g_CharSortAscending = !g_CharSortAscending;
                            std::string selName;
                            if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                                int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                                if (ri >= 0 && ri < (int)characters.size()) selName = characters[ri].name;
                            }
                            RebuildCharDisplayOrder();
                            g_SelectedCharIdx = 0;
                            for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                                if (characters[g_CharDisplayOrder[di]].name == selName) {
                                    g_SelectedCharIdx = di;
                                    break;
                                }
                            }
                            SaveCharSortConfig();
                        }
                        // Draw triangle arrow on the button
                        ImVec2 btnMin = ImGui::GetItemRectMin();
                        ImVec2 btnMax = ImGui::GetItemRectMax();
                        float cx = (btnMin.x + btnMax.x) * 0.5f;
                        float cy = (btnMin.y + btnMax.y) * 0.5f;
                        float sz = 5.0f;
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        ImU32 col = IM_COL32(220, 220, 220, 255);
                        if (g_CharSortAscending) {
                            // Up arrow
                            dl->AddTriangleFilled(
                                ImVec2(cx, cy - sz), ImVec2(cx - sz, cy + sz), ImVec2(cx + sz, cy + sz), col);
                        } else {
                            // Down arrow
                            dl->AddTriangleFilled(
                                ImVec2(cx - sz, cy - sz), ImVec2(cx + sz, cy - sz), ImVec2(cx, cy + sz), col);
                        }
                    }
                    ImGui::Separator();
                }

                // Character search bar
                {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::InputTextWithHint("##charsearch", "Search...", g_CharSearchBuf, sizeof(g_CharSearchBuf));
                }

                // Collect item rects for insertion-line drag-and-drop
                struct CharItemRect { float yMin, yMax; };
                std::vector<CharItemRect> itemRects;
                itemRects.reserve(g_CharDisplayOrder.size());

                // Cache which character is currently being fetched
                std::string fetchingCharName = AlterEgo::GW2API::GetCurrentFetchCharName();

                // Prepare lowercase search string
                std::string charSearchLower;
                if (g_CharSearchBuf[0] != '\0') {
                    charSearchLower = g_CharSearchBuf;
                    std::transform(charSearchLower.begin(), charSearchLower.end(),
                                   charSearchLower.begin(), ::tolower);
                }

                for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                    int realIdx = g_CharDisplayOrder[di];
                    if (realIdx < 0 || realIdx >= (int)characters.size()) continue;
                    const auto& ch = characters[realIdx];

                    // Apply search filter
                    if (!charSearchLower.empty()) {
                        std::string nameLower = ch.name;
                        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        std::string profLower = ch.profession;
                        std::transform(profLower.begin(), profLower.end(), profLower.begin(), ::tolower);
                        if (nameLower.find(charSearchLower) == std::string::npos &&
                            profLower.find(charSearchLower) == std::string::npos)
                            continue;
                    }
                    ImGui::PushID(di);

                    bool selected = (g_SelectedCharIdx == di);
                    bool isLoggedIn = (!g_CurrentCharName.empty() && ch.name == g_CurrentCharName);
                    ImVec4 profColor = GetProfessionColor(ch.profession);

                    // Count extra lines for dynamic row height
                    int extraLines = 0;
                    if (!g_CompactCharList) {
                        if (g_ShowCraftingIcons && !ch.crafting.empty()) extraLines++;
                        if (g_ShowAge && !ch.created.empty()) extraLines++;
                        if (g_ShowPlaytime && ch.age > 0) extraLines++;
                        if (g_ShowLastLogin && !ch.last_modified.empty()) extraLines++;
                        if (g_BirthdayMode != 2) {
                            int bdays = DaysUntilBirthday(ch.created);
                            bool showBday = (g_BirthdayMode == 0 && bdays >= 0) ||
                                            (g_BirthdayMode == 1 && bdays >= 0 && bdays <= 7);
                            if (showBday) extraLines++;
                        }
                    }
                    float lineH = ImGui::GetTextLineHeightWithSpacing();
                    float rowHeight = g_CompactCharList ? 18.0f
                        : (28.0f + extraLines * lineH);

                    // Green selection highlight for all characters
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.4f, 0.15f, 0.55f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.45f, 0.2f, 0.65f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.4f, 0.15f, 0.7f));

                    // Character entry
                    if (ImGui::Selectable("##char", selected, 0, ImVec2(0, rowHeight))) {
                        g_SelectedCharIdx = di;
                        g_DetailsFetched = false;
                        g_SelectedEquipTab = 0;
                        g_SelectedBuildTab = -1;
                    }

                    // Record item rect for insertion line calculation
                    ImVec2 rMin = ImGui::GetItemRectMin();
                    ImVec2 rMax = ImGui::GetItemRectMax();

                    itemRects.push_back({ rMin.y, rMax.y });

                    // Drag source for Custom sort mode
                    if (g_CharSortMode == Sort_Custom) {
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            g_CharDragIdx = di;
                            ImGui::SetDragDropPayload("CHAR_REORDER", &di, sizeof(int));
                            ImGui::TextColored(profColor, "%s", ch.name.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }

                    // Draw profession icon + character info on top of selectable
                    ImGui::SameLine(4);
                    ImGui::BeginGroup();
                    if (!g_CompactCharList) {
                        const float iconSz = 20.0f;
                        uint32_t profIconId = GetProfessionIconId(ch.profession);
                        const char* profIconUrl = GetProfessionIconUrl(ch.profession);
                        ImVec2 cpos = ImGui::GetCursorPos();
                        float yOff = (extraLines > 0) ? 2.0f : (rowHeight - iconSz) * 0.5f;
                        if (profIconId && profIconUrl) {
                            auto* tex = AlterEgo::IconManager::GetIcon(profIconId);
                            if (tex && tex->Resource) {
                                ImGui::SetCursorPos(ImVec2(cpos.x, cpos.y + yOff));
                                ImGui::Image(tex->Resource, ImVec2(iconSz, iconSz));
                            } else {
                                AlterEgo::IconManager::RequestIcon(profIconId, profIconUrl);
                                ImGui::SetCursorPos(ImVec2(cpos.x, cpos.y + yOff));
                                ImGui::Dummy(ImVec2(iconSz, iconSz));
                            }
                        } else {
                            ImGui::SetCursorPos(ImVec2(cpos.x, cpos.y + yOff));
                            ImGui::Dummy(ImVec2(iconSz, iconSz));
                        }
                        ImGui::SameLine(0, 4);
                        float textYOff = (extraLines > 0) ? 2.0f : (rowHeight - ImGui::GetTextLineHeight()) * 0.5f;
                        ImGui::SetCursorPosY(cpos.y + textYOff);
                    }
                    // First line: name + level + green dot
                    ImGui::TextColored(profColor, "%s", ch.name.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(selected ? ImVec4(0.75f, 0.75f, 0.75f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Lv%d", ch.level);
                    if (isLoggedIn) {
                        ImGui::SameLine();
                        float dotY = ImGui::GetCursorScreenPos().y - 3.0f + ImGui::GetTextLineHeight() * 0.5f;
                        float dotX = ImGui::GetCursorScreenPos().x;
                        ImGui::GetWindowDrawList()->AddCircleFilled(
                            ImVec2(dotX + 3.0f, dotY), 3.0f,
                            IM_COL32(60, 200, 80, 255));
                        ImGui::Dummy(ImVec2(8, 0));
                    }
                    // Spinning refresh icon for the character currently being fetched
                    if (!fetchingCharName.empty() && ch.name == fetchingCharName) {
                        ImGui::SameLine();
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        float iconR = 5.0f;
                        ImVec2 scrPos = ImGui::GetCursorScreenPos();
                        float cx = scrPos.x + iconR + 1.0f;
                        float cy = scrPos.y + ImGui::GetTextLineHeight() * 0.5f;
                        float angle = (float)ImGui::GetTime() * 3.0f; // spin speed
                        ImU32 col = IM_COL32(255, 220, 60, 220);
                        // Draw two arcs with arrowheads
                        for (int arc = 0; arc < 2; arc++) {
                            float arcStart = angle + arc * 3.14159f;
                            int segments = 8;
                            float arcSpan = 2.4f; // ~137 degrees per arc
                            for (int s = 0; s < segments; s++) {
                                float a0 = arcStart + (arcSpan * s / segments);
                                float a1 = arcStart + (arcSpan * (s + 1) / segments);
                                dl->AddLine(
                                    ImVec2(cx + cosf(a0) * iconR, cy + sinf(a0) * iconR),
                                    ImVec2(cx + cosf(a1) * iconR, cy + sinf(a1) * iconR),
                                    col, 1.5f);
                            }
                            // Arrowhead at end of arc
                            float aEnd = arcStart + arcSpan;
                            float tx = cosf(aEnd); // tangent direction (perpendicular to radius)
                            float ty = sinf(aEnd);
                            ImVec2 tip(cx + tx * iconR, cy + ty * iconR);
                            // Arrow points along the arc direction (tangent)
                            float tanX = -ty; // tangent = perpendicular to radial
                            float tanY = tx;
                            float arrSz = 3.0f;
                            // Two sides of the arrowhead
                            ImVec2 p1(tip.x - tanX * arrSz + tx * arrSz * 0.5f,
                                      tip.y - tanY * arrSz + ty * arrSz * 0.5f);
                            ImVec2 p2(tip.x - tanX * arrSz - tx * arrSz * 0.5f,
                                      tip.y - tanY * arrSz - ty * arrSz * 0.5f);
                            dl->AddTriangleFilled(tip, p1, p2, col);
                        }
                        ImGui::Dummy(ImVec2(iconR * 2.0f + 2.0f, 0));
                    }
                    // Extra lines (each on its own line with prefix)
                    if (!g_CompactCharList) {
                        ImVec4 dimCol = selected ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
                        ImVec4 labelCol = selected ? ImVec4(0.7f, 0.7f, 0.7f, 1.0f) : ImVec4(0.45f, 0.45f, 0.45f, 1.0f);

                        // 1. Crafting icons (no prefix, inactive dimmed)
                        if (g_ShowCraftingIcons && !ch.crafting.empty()) {
                            const float craftIconSz = 14.0f;
                            for (size_t ci = 0; ci < ch.crafting.size(); ci++) {
                                if (ci > 0) ImGui::SameLine(0, 2);
                                const std::string& disc = ch.crafting[ci];
                                bool isActive = (ci < ch.crafting_active.size()) ? ch.crafting_active[ci] : true;
                                float iconAlpha = isActive ? 1.0f : 0.35f;
                                uint32_t cIconId = GetCraftingIconId(disc);
                                const char* cIconUrl = GetCraftingIconUrl(disc);
                                bool rendered = false;
                                if (cIconId && cIconUrl) {
                                    auto* tex = AlterEgo::IconManager::GetIcon(cIconId);
                                    if (tex && tex->Resource) {
                                        ImGui::Image(tex->Resource, ImVec2(craftIconSz, craftIconSz),
                                            ImVec2(0, 0), ImVec2(1, 1),
                                            ImVec4(1, 1, 1, iconAlpha));
                                        rendered = true;
                                    } else {
                                        AlterEgo::IconManager::RequestIcon(cIconId, cIconUrl);
                                    }
                                }
                                if (!rendered) ImGui::Dummy(ImVec2(craftIconSz, craftIconSz));
                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    int lvl = (ci < ch.crafting_levels.size()) ? ch.crafting_levels[ci] : 0;
                                    ImGui::Text("%s %d%s", disc.c_str(), lvl,
                                        isActive ? "" : " (inactive)");
                                    ImGui::EndTooltip();
                                }
                            }
                        }

                        // 2. Age: (character creation age)
                        if (g_ShowAge && !ch.created.empty()) {
                            struct tm ctm = {};
                            if (sscanf(ch.created.c_str(), "%d-%d-%dT%d:%d:%d",
                                    &ctm.tm_year, &ctm.tm_mon, &ctm.tm_mday,
                                    &ctm.tm_hour, &ctm.tm_min, &ctm.tm_sec) == 6) {
                                ctm.tm_year -= 1900;
                                ctm.tm_mon -= 1;
                                time_t created_t = mktime(&ctm);
                                time_t now_t = std::time(nullptr);
                                int totalDays = (int)(difftime(now_t, created_t) / 86400.0);
                                ImGui::TextColored(labelCol, "Age:");
                                ImGui::SameLine();
                                if (totalDays >= 365)
                                    ImGui::TextColored(dimCol, "%dy %dd", totalDays / 365, totalDays % 365);
                                else
                                    ImGui::TextColored(dimCol, "%dd", totalDays);
                            }
                        }

                        // 3. Next Birthday: (right after Age)
                        if (g_BirthdayMode != 2) {
                            int bdays = DaysUntilBirthday(ch.created);
                            bool showBday = (g_BirthdayMode == 0 && bdays >= 0) ||
                                            (g_BirthdayMode == 1 && bdays >= 0 && bdays <= 7);
                            if (showBday) {
                                ImGui::TextColored(labelCol, "Next Birthday:");
                                ImGui::SameLine();
                                if (bdays == 0)
                                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Today!");
                                else
                                    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "%dd", bdays);
                            }
                        }

                        // 4. Playtime:
                        if (g_ShowPlaytime && ch.age > 0) {
                            int hours = ch.age / 3600;
                            ImGui::TextColored(labelCol, "Playtime:");
                            ImGui::SameLine();
                            if (hours >= 24)
                                ImGui::TextColored(dimCol, "%dd %dh", hours / 24, hours % 24);
                            else
                                ImGui::TextColored(dimCol, "%dh", hours);
                        }

                        // 5. Last Login:
                        if (g_ShowLastLogin && !ch.last_modified.empty()) {
                            struct tm tm = {};
                            if (sscanf(ch.last_modified.c_str(), "%d-%d-%dT%d:%d:%d",
                                    &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                                    &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
                                tm.tm_year -= 1900;
                                tm.tm_mon -= 1;
                                time_t login = mktime(&tm);
                                time_t now_t = std::time(nullptr);
                                int elapsed = (int)difftime(now_t, login);
                                std::string ago;
                                if (elapsed < 3600) ago = std::to_string(elapsed / 60) + "m";
                                else if (elapsed < 86400) ago = std::to_string(elapsed / 3600) + "h";
                                else ago = std::to_string(elapsed / 86400) + "d";
                                ImGui::TextColored(labelCol, "Last Login:");
                                ImGui::SameLine();
                                ImGui::TextColored(dimCol, "%s ago", ago.c_str());
                            }
                        }
                    }
                    ImGui::EndGroup();

                    ImGui::PopStyleColor(3);

                    ImGui::PopID();
                }

                // Custom sort: draw insertion line and handle drop
                if (g_CharSortMode == Sort_Custom && g_CharDragIdx >= 0 &&
                    ImGui::GetDragDropPayload() != nullptr) {
                    float mouseY = ImGui::GetMousePos().y;
                    int insertIdx = (int)itemRects.size(); // default: end
                    float bestLineY = 0;

                    for (int di = 0; di < (int)itemRects.size(); di++) {
                        float midY = (itemRects[di].yMin + itemRects[di].yMax) * 0.5f;
                        if (mouseY < midY) {
                            insertIdx = di;
                            bestLineY = itemRects[di].yMin;
                            break;
                        }
                    }
                    if (insertIdx == (int)itemRects.size() && !itemRects.empty())
                        bestLineY = itemRects.back().yMax;

                    // Draw insertion line
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    float xMin = ImGui::GetWindowPos().x + 2;
                    float xMax = xMin + ImGui::GetWindowContentRegionMax().x - 4;
                    dl->AddLine(ImVec2(xMin, bestLineY), ImVec2(xMax, bestLineY),
                        IM_COL32(100, 180, 255, 220), 2.0f);

                    // Handle drop anywhere in the list area
                    if (ImGui::IsMouseReleased(0)) {
                        int srcDi = g_CharDragIdx;
                        // Adjust insert index if dragging from before the insertion point
                        int targetDi = insertIdx;
                        if (srcDi < targetDi) targetDi--;
                        if (srcDi != targetDi && srcDi >= 0 && srcDi < (int)g_CharDisplayOrder.size()) {
                            int movedIdx = g_CharDisplayOrder[srcDi];
                            g_CharDisplayOrder.erase(g_CharDisplayOrder.begin() + srcDi);
                            int finalPos = (srcDi < insertIdx) ? insertIdx - 1 : insertIdx;
                            if (finalPos > (int)g_CharDisplayOrder.size())
                                finalPos = (int)g_CharDisplayOrder.size();
                            g_CharDisplayOrder.insert(g_CharDisplayOrder.begin() + finalPos, movedIdx);
                            // Update custom order
                            g_CustomCharOrder.clear();
                            for (int idx : g_CharDisplayOrder)
                                g_CustomCharOrder.push_back(characters[idx].name);
                            // Fix selection
                            if (g_SelectedCharIdx == srcDi)
                                g_SelectedCharIdx = finalPos;
                            else if (srcDi < finalPos && g_SelectedCharIdx > srcDi && g_SelectedCharIdx <= finalPos)
                                g_SelectedCharIdx--;
                            else if (srcDi > finalPos && g_SelectedCharIdx >= finalPos && g_SelectedCharIdx < srcDi)
                                g_SelectedCharIdx++;
                            SaveCharSortConfig();
                        }
                        g_CharDragIdx = -1;
                    }
                } else if (ImGui::IsMouseReleased(0)) {
                    g_CharDragIdx = -1;
                }

                ImGui::EndChild();

                ImGui::SameLine();

                // Detail panel — resolve through display order
                int selRealIdx = -1;
                if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size())
                    selRealIdx = g_CharDisplayOrder[g_SelectedCharIdx];

                ImGui::BeginChild("CharDetail", ImVec2(0, 0), true);
                if (selRealIdx >= 0 && selRealIdx < (int)characters.size()) {
                    const auto& ch = characters[selRealIdx];

                    // Fetch details on-demand when character is selected
                    if (!g_DetailsFetched) {
                        FetchDetailsForCharacter(ch);
                        g_DetailsFetched = true;
                    }

                    // Character header
                    ImVec4 profColor = GetProfessionColor(ch.profession);
                    ImGui::TextColored(profColor, "%s", ch.name.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                        "Level %d %s %s", ch.level, ch.race.c_str(), ch.profession.c_str());

                    // Birthday countdown
                    {
                        int bdays = DaysUntilBirthday(ch.created);
                        int age = CharacterAgeYears(ch.created);
                        if (bdays >= 0 && age >= 0) {
                            if (bdays == 0) {
                                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
                                    "Happy Birthday! Turning %d today!", age + 1);
                            } else {
                                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                    "Age %d  -  Birthday in %d day%s", age, bdays, bdays == 1 ? "" : "s");
                            }
                        }
                    }

                    // Last login
                    if (!ch.last_modified.empty()) {
                        // Parse ISO date to time_t for relative display
                        struct tm tm_val = {};
                        if (sscanf(ch.last_modified.c_str(), "%d-%d-%dT%d:%d:%d",
                            &tm_val.tm_year, &tm_val.tm_mon, &tm_val.tm_mday,
                            &tm_val.tm_hour, &tm_val.tm_min, &tm_val.tm_sec) >= 3) {
                            tm_val.tm_year -= 1900;
                            tm_val.tm_mon -= 1;
                            time_t login_t = _mkgmtime(&tm_val);
                            time_t now_t = std::time(nullptr);
                            int elapsed = (int)difftime(now_t, login_t);
                            std::string ago;
                            if (elapsed < 60) ago = "just now";
                            else if (elapsed < 3600) ago = std::to_string(elapsed / 60) + "m ago";
                            else if (elapsed < 86400) ago = std::to_string(elapsed / 3600) + "h ago";
                            else ago = std::to_string(elapsed / 86400) + "d ago";
                            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Last login: %s", ago.c_str());
                        }
                    }

                    // Crafting disciplines
                    if (!ch.crafting.empty()) {
                        ImGui::SameLine(0, 20);
                        std::string craftStr;
                        for (size_t ci = 0; ci < ch.crafting.size(); ci++) {
                            if (ci > 0) craftStr += "  ";
                            craftStr += ch.crafting[ci];
                            if (ci < ch.crafting_levels.size())
                                craftStr += " " + std::to_string(ch.crafting_levels[ci]);
                        }
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", craftStr.c_str());
                    }

                    ImGui::Separator();

                    // Tab bar: Equipment | Build
                    if (ImGui::BeginTabBar("##detail_tabs")) {
                        if (ImGui::BeginTabItem("Equipment")) {
                            g_SelectedTab = 0;
                            ImGui::BeginChild("EquipScroll", ImVec2(0, 0), false);
                            RenderEquipmentPanel(ch);
                            ImGui::EndChild();
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Build")) {
                            g_SelectedTab = 1;
                            ImGui::BeginChild("BuildScroll", ImVec2(0, 0), false);
                            RenderBuildPanel(ch);
                            ImGui::EndChild();
                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a character from the list.");
                }
                ImGui::EndChild();
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Build Library")) {
            g_MainTab = 1;
            RenderBuildLibrary();
            ImGui::EndTabItem();
        }

        if (g_SkinInitialized && ImGui::BeginTabItem("Skinventory")) {
            g_MainTab = 2;
            RenderSkinventory();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Clears")) {
            g_MainTab = 3;
            RenderClears();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

// --- Options/Settings Render ---

void AddonOptions() {
    // Header with links
    ImGui::Text("Alter Ego");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
    ImGui::SameLine();
    if (ImGui::SmallButton("GitHub")) {
        ShellExecuteA(NULL, "open", "https://github.com/PieOrCake/alter-ego", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
    ImGui::SameLine();
    if (ImGui::SmallButton("Ko-fi")) {
        ShellExecuteA(NULL, "open", "https://ko-fi.com/pieorcake", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::Separator();

    // H&S connection status
    ImGui::Text("Data Source: Hoard & Seek");
    auto hoardStatus = AlterEgo::GW2API::GetHoardStatus();
    switch (hoardStatus) {
        case AlterEgo::HoardStatus::Unknown:
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status: Checking...");
            break;
        case AlterEgo::HoardStatus::Unavailable:
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Status: Not detected");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Hoard & Seek is required. Install it from the Nexus addon library.");
            break;
        case AlterEgo::HoardStatus::Available:
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Status: Connected");
            break;
        case AlterEgo::HoardStatus::PermPending:
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Status: Permission pending");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Accept the Alter Ego permission popup in Hoard & Seek.");
            break;
        case AlterEgo::HoardStatus::PermDenied:
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Status: Permission denied");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Re-enable in H&S settings under Permissions.");
            if (ImGui::SmallButton("Retry Connection")) {
                AlterEgo::GW2API::PingHoard();
            }
            break;
        case AlterEgo::HoardStatus::Ready:
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Status: Ready");
            break;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("UI Settings:");
    if (ImGui::Checkbox("Show Quick Access icon", &g_ShowQAIcon)) {
        if (g_ShowQAIcon) {
            APIDefs->QuickAccess_Add(QA_ID, TEX_ICON, TEX_ICON_HOVER, "KB_ALTER_EGO_TOGGLE", "Alter Ego");
        } else {
            APIDefs->QuickAccess_Remove(QA_ID);
        }
        SaveSettings();
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Character List:");
    if (ImGui::Checkbox("Compact mode", &g_CompactCharList)) {
        SaveSettings();
    }
    if (!g_CompactCharList) {
        ImGui::Indent(16.0f);
        if (ImGui::Checkbox("Crafting Profession Icons", &g_ShowCraftingIcons)) {
            SaveSettings();
        }
        if (ImGui::Checkbox("Age", &g_ShowAge)) {
            SaveSettings();
        }
        if (ImGui::Checkbox("Playtime", &g_ShowPlaytime)) {
            SaveSettings();
        }
        if (ImGui::Checkbox("Last Login", &g_ShowLastLogin)) {
            SaveSettings();
        }
        ImGui::Text("Next Birthday");
        ImGui::SameLine();
        if (ImGui::RadioButton("Always", g_BirthdayMode == 0)) {
            g_BirthdayMode = 0;
            SaveSettings();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("A week out", g_BirthdayMode == 1)) {
            g_BirthdayMode = 1;
            SaveSettings();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Never", g_BirthdayMode == 2)) {
            g_BirthdayMode = 2;
            SaveSettings();
        }
        ImGui::Unindent(16.0f);
    }
}

// --- Export: GetAddonDef ---

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef() {
    AddonDef.Signature = 0xA17E3E90;  // Unique ID for Alter Ego
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "Alter Ego";
    AddonDef.Version.Major = V_MAJOR;
    AddonDef.Version.Minor = V_MINOR;
    AddonDef.Version.Build = V_BUILD;
    AddonDef.Version.Revision = V_REVISION;
    AddonDef.Author = "PieOrCake.7635";
    AddonDef.Description = "Character & build manager";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = AF_None;
    AddonDef.Provider = UP_GitHub;
    AddonDef.UpdateLink = nullptr;

    return &AddonDef;
}
