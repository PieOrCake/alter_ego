#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <atomic>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <set>

#include "imgui.h"
#include <nlohmann/json.hpp>
#include "GW2API.h"
#include "HoardAndSeekAPI.h"
#include "HttpClient.h"
#include "AddonShared.h"
#include "Clears.h"
#include "WikiImage.h"

// =========================================================================
// Clears - Data types and module-private state
// =========================================================================

// Achievement category IDs
static constexpr uint32_t CAT_DAILY_FRACTALS = 88;
static constexpr uint32_t CAT_DAILY_BOUNTIES = 475;
static constexpr uint32_t CAT_WEEKLY_RAIDS   = 477;
static constexpr uint32_t ACH_WEEKLY_STRIKES = 9125;

struct ClearEntry {
    uint32_t id = 0;
    std::string name;
    std::string tier;
    bool done = false;
    int32_t current = 0;
    int32_t max = 0;
    std::vector<std::string> bitNames;
    std::vector<bool> bitDone;
};

// Module-private state (not accessible outside this TU)
static std::vector<ClearEntry> g_DailyFractals;
static std::vector<ClearEntry> g_DailyBounties;
static std::vector<ClearEntry> g_WeeklyWings;
static ClearEntry g_WeeklyStrikes;
static std::mutex g_ClearsMutex;
static std::string g_ClearsStatusMsg;
static std::chrono::system_clock::time_point g_LastDailyReset{};
static std::chrono::system_clock::time_point g_LastWeeklyReset{};
static std::chrono::steady_clock::time_point g_LastClearsCompletionQuery{};
static std::atomic<bool> g_ClearsRetryPending{false};

// Public globals — declared extern in Clears.h, accessed from dllmain.cpp
bool g_ClearsFetched = false;
bool g_ClearsFetching = false;
bool g_ClearsNeedRequery = false;
std::atomic<bool> g_ClearsQueryPending{false};

// ---- Wizard's Vault state ----
struct VaultObjective {
    uint32_t id = 0;
    std::string title;
    std::string track;  // "PvE", "PvP", "WvW"
    int32_t acclaim = 0;
    int32_t progress_current = 0;
    int32_t progress_complete = 0;
    bool claimed = false;
    bool isDone() const { return progress_complete > 0 && progress_current >= progress_complete; }
};

struct VaultPeriod {
    int32_t meta_progress_current = 0;
    int32_t meta_progress_complete = 0;
    int32_t meta_reward_astral = 0;
    bool meta_reward_claimed = false;
    std::vector<VaultObjective> objectives;
};

static VaultPeriod g_VaultDaily;
static VaultPeriod g_VaultWeekly;
static VaultPeriod g_VaultSpecial;
static bool g_VaultFetched = false;
static bool g_VaultFetching = false;
static std::atomic<bool> g_VaultRetryPending{false};
bool g_VaultNeedRequery = false;
static std::chrono::system_clock::time_point g_VaultLastDailyReset{};
static std::chrono::system_clock::time_point g_VaultLastWeeklyReset{};
static std::chrono::system_clock::time_point g_VaultSeasonEnd{};

// =========================================================================
// Clears - Backend (reset calculations, fetch, H&S response)
// =========================================================================

// Forward declarations of private helpers
static void SaveClearsCache();
static void FetchClears();
static void SendClearsAchQuery();
static void FetchVaultData();
static void SaveVaultCache();
static void RenderClearsTabContent(const std::string& dailyResetStr, const std::string& weeklyResetStr);
static void RenderVaultPeriodSection(const char* title, const char* resetLabel, ImVec4 color, const VaultPeriod& period);
static void RenderVaultTab(const std::string& dailyResetStr, const std::string& weeklyResetStr);

// Calculate the most recent daily reset (00:00 UTC) before 'now'
std::chrono::system_clock::time_point CalcLastDailyReset(std::chrono::system_clock::time_point now) {
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
std::chrono::system_clock::time_point CalcLastWeeklyReset(std::chrono::system_clock::time_point now) {
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
    int d = (int)(remaining / 86400);
    int h = (int)((remaining % 86400) / 3600);
    int m = (int)((remaining % 3600) / 60);
    if (d > 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%dd %dh", d, h);
        return buf;
    }
    if (h > 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%dh %dm", h, m);
        return buf;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%dm", m);
    return buf;
}

static std::chrono::system_clock::time_point ParseISO8601(const std::string& s) {
    int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;
    if (sscanf(s.c_str(), "%d-%d-%dT%d:%d:%d", &year, &mon, &day, &hour, &min, &sec) != 6)
        return {};
    struct tm t{};
    t.tm_year = year - 1900; t.tm_mon = mon - 1; t.tm_mday = day;
    t.tm_hour = hour; t.tm_min = min; t.tm_sec = sec;
#ifdef _WIN32
    return std::chrono::system_clock::from_time_t(_mkgmtime(&t));
#else
    return std::chrono::system_clock::from_time_t(timegm(&t));
#endif
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

    std::string acctName = GetEffectiveAccountName();
    HoardQueryAchievementRequest req{};
    req.api_version = HOARD_API_VERSION;
    strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
    strncpy(req.response_event, EV_AE_CLEARS_ACH_RESPONSE, sizeof(req.response_event) - 1);
    if (!acctName.empty())
        strncpy(req.account_name, acctName.c_str(), sizeof(req.account_name) - 1);
    req.id_count = (uint32_t)std::min(allIds.size(), (size_t)200);
    for (uint32_t i = 0; i < req.id_count; i++) {
        req.ids[i] = allIds[i];
    }
    APIDefs->Events_Raise(EV_HOARD_QUERY_ACHIEVEMENT, &req);
}

// H&S achievement response handler for all clears
void OnClearsAchResponse(void* eventArgs) {
    if (!eventArgs) return;
    auto* resp = (HoardQueryAchievementResponse*)eventArgs;
    if (resp->api_version < 2) { return; }

    if (resp->status != HOARD_STATUS_OK) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        if (resp->status == HOARD_STATUS_PENDING) {
            g_ClearsStatusMsg = "Waiting for H&S permission...";
            // Signal render thread to retry (Events_Raise must be on render thread)
            g_ClearsRetryPending = true;
        } else {
            if (resp->status == HOARD_STATUS_DENIED)
                g_ClearsStatusMsg = "H&S permission denied";
            else
                g_ClearsStatusMsg = "H&S error";
            g_ClearsFetching = false;
        }
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
                // API clears bits when achievement is fully completed;
                // detect this and mark all encounters done
                if (it->second.done && ce.max > 0 && ce.current >= ce.max) {
                    ce.bitDone.assign(ce.bitNames.size(), true);
                } else {
                    ce.bitDone.assign(ce.bitNames.size(), false);
                    for (uint32_t bitIdx : it->second.bits) {
                        if (bitIdx < ce.bitDone.size()) {
                            ce.bitDone[bitIdx] = true;
                        }
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
}

// =========================================================================
// Clears - Wizard's Vault fetch and response
// =========================================================================

static void FetchVaultData() {
    if (g_VaultFetching || !APIDefs) return;
    g_VaultFetching = true;
    g_VaultFetched = false;

    std::string acctName = GetEffectiveAccountName();
    // H&S dispatches synchronously — responses handled inline inside Events_Raise
    for (uint8_t type = 0; type <= 2; type++) {
        HoardQueryWizardsVaultRequest req{};
        req.api_version = HOARD_API_VERSION;
        strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
        strncpy(req.response_event, EV_AE_VAULT_RESPONSE, sizeof(req.response_event) - 1);
        req.type = type;
        if (!acctName.empty())
            strncpy(req.account_name, acctName.c_str(), sizeof(req.account_name) - 1);
        APIDefs->Events_Raise(EV_HOARD_QUERY_WIZARDSVAULT, &req);
    }

    // Also fetch season info (for the special objectives end date)
    {
        HoardQueryApiRequest req{};
        req.api_version = HOARD_API_VERSION;
        strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
        strncpy(req.endpoint, "/v2/account/wizardsvault", sizeof(req.endpoint) - 1);
        strncpy(req.response_event, EV_AE_VAULT_SEASON_RESP, sizeof(req.response_event) - 1);
        if (!acctName.empty())
            strncpy(req.account_name, acctName.c_str(), sizeof(req.account_name) - 1);
        APIDefs->Events_Raise(EV_HOARD_QUERY_API, &req);
    }

    g_VaultFetching = false;
    g_VaultFetched = true;
    SaveVaultCache();
}

void OnVaultSeasonResponse(void* eventArgs) {
    if (!eventArgs) return;
    auto* resp = static_cast<HoardQueryApiResponse*>(eventArgs);
    if (resp->status != HOARD_STATUS_OK || resp->json_length == 0) return;
    try {
        auto j = nlohmann::json::parse(resp->json);
        std::string endStr = j.value("end", "");
        if (!endStr.empty()) {
            auto tp = ParseISO8601(endStr);
            if (tp != std::chrono::system_clock::time_point{}) {
                std::lock_guard<std::mutex> lock(g_ClearsMutex);
                g_VaultSeasonEnd = tp;
            }
        }
    } catch (...) {}
}

void OnVaultResponse(void* eventArgs) {
    if (!eventArgs) return;
    auto* resp = (HoardQueryWizardsVaultResponse*)eventArgs;
    if (resp->api_version < 2) return;

    if (resp->status != HOARD_STATUS_OK) {
        if (resp->status == HOARD_STATUS_PENDING)
            g_VaultRetryPending = true;
        return;
    }

    VaultPeriod period;
    period.meta_progress_current  = resp->meta_progress_current;
    period.meta_progress_complete = resp->meta_progress_complete;
    period.meta_reward_astral     = resp->meta_reward_astral;
    period.meta_reward_claimed    = resp->meta_reward_claimed != 0;
    for (uint32_t i = 0; i < resp->objective_count && i < 16; i++) {
        const auto& o = resp->objectives[i];
        VaultObjective obj;
        obj.id                = o.id;
        obj.title             = o.title;
        obj.track             = o.track;
        obj.acclaim           = o.acclaim;
        obj.progress_current  = o.progress_current;
        obj.progress_complete = o.progress_complete;
        obj.claimed           = o.claimed != 0;
        period.objectives.push_back(std::move(obj));
    }

    std::lock_guard<std::mutex> lock(g_ClearsMutex);
    switch (resp->type) {
        case 0: g_VaultDaily   = std::move(period); break;
        case 1: g_VaultWeekly  = std::move(period); break;
        case 2: g_VaultSpecial = std::move(period); break;
    }
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

        // Sort wings into canonical W1-W8 order by achievement ID
        static const std::unordered_map<uint32_t, int> wingOrder = {
            {9128, 1}, // Spirit Vale
            {9147, 2}, // Salvation Pass
            {9182, 3}, // Stronghold of the Faithful
            {9144, 4}, // Bastion of the Penitent
            {9111, 5}, // Hall of Chains
            {9120, 6}, // Mythwright Gambit
            {9156, 7}, // Key of Ahdashim
            {9181, 8}, // Mount Balrior
        };
        std::sort(wingEntries.begin(), wingEntries.end(),
            [](const ClearEntry& a, const ClearEntry& b) {
                auto ai = wingOrder.find(a.id);
                auto bi = wingOrder.find(b.id);
                int ao = (ai != wingOrder.end()) ? ai->second : 99;
                int bo = (bi != wingOrder.end()) ? bi->second : 99;
                return ao < bo;
            });

        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_DailyFractals = std::move(fractalEntries);
            g_DailyBounties = std::move(bountyEntries);
            g_WeeklyWings = std::move(wingEntries);
            g_WeeklyStrikes = std::move(strikesEntry);
            g_ClearsStatusMsg = "Querying completion...";
        }

        // Background fetch done — allow render thread to send H&S completion query
        g_ClearsFetching = false;
        g_ClearsQueryPending = true;
    }).detach();
}

// =========================================================================
// Clears - Persistence
// =========================================================================

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

    // Snapshot data under lock to prevent concurrent modification
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);

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
    }

    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

void LoadClearsCache() {
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
        // Add 60s buffer so we don't treat cache as stale before the API has updated
        auto now = std::chrono::system_clock::now();
        auto dailyReset = CalcLastDailyReset(now) + std::chrono::seconds(60);
        auto weeklyReset = CalcLastWeeklyReset(now) + std::chrono::seconds(60);
        bool dailyStale = fetchTime < dailyReset && now >= dailyReset;
        bool weeklyStale = fetchTime < weeklyReset && now >= weeklyReset;

        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);

            // Clear vectors before loading to prevent accumulation
            g_DailyFractals.clear();
            g_DailyBounties.clear();
            g_WeeklyWings.clear();
            g_WeeklyStrikes = ClearEntry{};

            // Helper: deduplicate entries by ID
            auto dedup = [](std::vector<ClearEntry>& vec) {
                std::unordered_set<uint32_t> seen;
                vec.erase(std::remove_if(vec.begin(), vec.end(),
                    [&seen](const ClearEntry& e) {
                        return !seen.insert(e.id).second;
                    }), vec.end());
            };

            // Load daily data only if no daily reset has occurred since fetch
            if (!dailyStale) {
                if (j.contains("dailyFractals") && j["dailyFractals"].is_array()) {
                    for (const auto& ej : j["dailyFractals"])
                        g_DailyFractals.push_back(deserializeEntry(ej));
                    dedup(g_DailyFractals);
                }
                if (j.contains("dailyBounties") && j["dailyBounties"].is_array()) {
                    for (const auto& ej : j["dailyBounties"])
                        g_DailyBounties.push_back(deserializeEntry(ej));
                    dedup(g_DailyBounties);
                }
            }

            // Load weekly data only if no weekly reset has occurred since fetch
            if (!weeklyStale) {
                if (j.contains("weeklyWings") && j["weeklyWings"].is_array()) {
                    for (const auto& ej : j["weeklyWings"])
                        g_WeeklyWings.push_back(deserializeEntry(ej));
                    dedup(g_WeeklyWings);
                    // Sort into canonical W1-W8 order
                    static const std::unordered_map<uint32_t, int> wingOrder = {
                        {9128, 1}, {9147, 2}, {9182, 3}, {9144, 4},
                        {9111, 5}, {9120, 6}, {9156, 7}, {9181, 8},
                    };
                    std::sort(g_WeeklyWings.begin(), g_WeeklyWings.end(),
                        [](const ClearEntry& a, const ClearEntry& b) {
                            auto ai = wingOrder.find(a.id);
                            auto bi = wingOrder.find(b.id);
                            int ao = (ai != wingOrder.end()) ? ai->second : 99;
                            int bo = (bi != wingOrder.end()) ? bi->second : 99;
                            return ao < bo;
                        });
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

// =========================================================================
// Clears - Wizard's Vault persistence
// =========================================================================

static void SaveVaultCache() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/vault_cache.json";

    auto serializePeriod = [](const VaultPeriod& p) -> nlohmann::json {
        nlohmann::json j;
        j["meta_progress_current"]  = p.meta_progress_current;
        j["meta_progress_complete"] = p.meta_progress_complete;
        j["meta_reward_astral"]     = p.meta_reward_astral;
        j["meta_reward_claimed"]    = p.meta_reward_claimed;
        nlohmann::json objs = nlohmann::json::array();
        for (const auto& o : p.objectives) {
            nlohmann::json oj;
            oj["id"]               = o.id;
            oj["title"]            = o.title;
            oj["track"]            = o.track;
            oj["acclaim"]          = o.acclaim;
            oj["progress_current"] = o.progress_current;
            oj["progress_complete"]= o.progress_complete;
            oj["claimed"]          = o.claimed;
            objs.push_back(std::move(oj));
        }
        j["objectives"] = std::move(objs);
        return j;
    };

    auto epochSecs = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    nlohmann::json j;
    j["fetch_time"] = epochSecs;
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        j["daily"]      = serializePeriod(g_VaultDaily);
        j["weekly"]     = serializePeriod(g_VaultWeekly);
        j["special"]    = serializePeriod(g_VaultSpecial);
        j["season_end"] = std::chrono::duration_cast<std::chrono::seconds>(
            g_VaultSeasonEnd.time_since_epoch()).count();
    }

    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

void LoadVaultCache() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/vault_cache.json";
    std::ifstream file(path);
    if (!file.is_open()) return;

    auto deserializePeriod = [](const nlohmann::json& j) -> VaultPeriod {
        VaultPeriod p;
        p.meta_progress_current  = j.value("meta_progress_current", 0);
        p.meta_progress_complete = j.value("meta_progress_complete", 0);
        p.meta_reward_astral     = j.value("meta_reward_astral", 0);
        p.meta_reward_claimed    = j.value("meta_reward_claimed", false);
        if (j.contains("objectives") && j["objectives"].is_array()) {
            for (const auto& oj : j["objectives"]) {
                VaultObjective o;
                o.id                = oj.value("id", 0u);
                o.title             = oj.value("title", "");
                o.track             = oj.value("track", "");
                o.acclaim           = oj.value("acclaim", 0);
                o.progress_current  = oj.value("progress_current", 0);
                o.progress_complete = oj.value("progress_complete", 0);
                o.claimed           = oj.value("claimed", false);
                p.objectives.push_back(std::move(o));
            }
        }
        return p;
    };

    try {
        auto j = nlohmann::json::parse(file);
        int64_t fetchEpoch = j.value("fetch_time", (int64_t)0);
        auto fetchTime = std::chrono::system_clock::from_time_t((time_t)fetchEpoch);

        auto now         = std::chrono::system_clock::now();
        auto dailyReset  = CalcLastDailyReset(now)  + std::chrono::seconds(60);
        auto weeklyReset = CalcLastWeeklyReset(now) + std::chrono::seconds(60);
        bool dailyStale  = fetchTime < dailyReset  && now >= dailyReset;
        bool weeklyStale = fetchTime < weeklyReset && now >= weeklyReset;

        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            if (!dailyStale  && j.contains("daily"))   g_VaultDaily   = deserializePeriod(j["daily"]);
            if (!weeklyStale && j.contains("weekly"))  g_VaultWeekly  = deserializePeriod(j["weekly"]);
            if (j.contains("special"))                 g_VaultSpecial = deserializePeriod(j["special"]);
            int64_t seasonEndEpoch = j.value("season_end", (int64_t)0);
            if (seasonEndEpoch > 0)
                g_VaultSeasonEnd = std::chrono::system_clock::from_time_t((time_t)seasonEndEpoch);

            if (!g_VaultDaily.objectives.empty() || !g_VaultWeekly.objectives.empty())
                g_VaultFetched = true;
        }

        // Stale data will be re-fetched on first render of the vault tab
        if (dailyStale || weeklyStale)
            g_VaultFetched = false;

    } catch (...) {}
}

// =========================================================================
// Clears - UI rendering helpers
// =========================================================================

// Scale → fractal map (per https://wiki.guildwars2.com/wiki/Fractals_of_the_Mists#Scales)
static const char* FractalNameForScale(int scale) {
    static const char* kScaleMap[101] = {
        nullptr,
        "Volcanic", "Uncategorized", "Snowblind", "Urban Battleground", "Swampland",
        "Cliffside", "Aquatic Ruins", "Underground Facility", "Molten Furnace", "Molten Boss",
        "Deepstone", "Siren's Reef", "Chaos", "Aetherblade", "Thaumanova Reactor",
        "Twilight Oasis", "Kinfall", "Captain Mai Trin Boss", "Volcanic", "Solid Ocean",
        "Silent Surf", "Nightmare", "Shattered Observatory", "Sunqua Peak", "Lonely Tower",
        "Aquatic Ruins", "Snowblind", "Volcanic", "Underground Facility", "Chaos",
        "Urban Battleground", "Swampland", "Deepstone", "Thaumanova Reactor", "Solid Ocean",
        "Uncategorized", "Siren's Reef", "Kinfall", "Molten Furnace", "Molten Boss",
        "Twilight Oasis", "Captain Mai Trin Boss", "Silent Surf", "Solid Ocean", "Aetherblade",
        "Cliffside", "Nightmare", "Shattered Observatory", "Sunqua Peak", "Lonely Tower",
        "Snowblind", "Volcanic", "Underground Facility", "Siren's Reef", "Thaumanova Reactor",
        "Swampland", "Urban Battleground", "Molten Furnace", "Twilight Oasis", "Solid Ocean",
        "Aquatic Ruins", "Uncategorized", "Chaos", "Thaumanova Reactor", "Aetherblade",
        "Silent Surf", "Deepstone", "Cliffside", "Molten Boss", "Kinfall",
        "Captain Mai Trin Boss", "Nightmare", "Shattered Observatory", "Sunqua Peak", "Lonely Tower",
        "Aquatic Ruins", "Swampland", "Siren's Reef", "Uncategorized", "Solid Ocean",
        "Underground Facility", "Thaumanova Reactor", "Molten Furnace", "Deepstone", "Urban Battleground",
        "Snowblind", "Twilight Oasis", "Chaos", "Swampland", "Molten Boss",
        "Captain Mai Trin Boss", "Volcanic", "Aetherblade", "Cliffside", "Kinfall",
        "Nightmare", "Shattered Observatory", "Sunqua Peak", "Silent Surf", "Lonely Tower",
    };
    if (scale < 1 || scale > 100) return nullptr;
    return kScaleMap[scale];
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
        std::string tail = name.substr(start);
        // "Scale 31" → "Swampland (31)"
        if (tail.rfind("Scale ", 0) == 0) {
            int scale = atoi(tail.c_str() + 6);
            if (const char* mapName = FractalNameForScale(scale)) {
                char buf[96];
                snprintf(buf, sizeof(buf), "%s (%d)", mapName, scale);
                return std::string(buf);
            }
        }
        return tail;
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

// Map verbose API encounter text to readable short names
static std::string ShortenEncounterName(const std::string& text) {
    static const std::unordered_map<std::string, std::string> nameMap = {
        // W1 — Spirit Vale
        {"Defeat the Vale Guardian.",                                      "Vale Guardian"},
        {"Traverse the Spirit Woods.",                                     "Spirit Woods"},
        {"Destroy Gorseval.",                                              "Gorseval"},
        {"Cull the bandits to lure out the bandit leader.",                ""},
        {"Defeat Sabetha the Saboteur.",                                   "Sabetha"},
        // W2 — Salvation Pass
        {"Defeat Slothasor.",                                              "Sloth"},
        {"Protect the caged prisoners.",                                   "Trio"},
        {"Cull the bandits in the ruins.",                                 ""},
        {"Defeat Inquisitor Matthias Gabrel.",                             "Matthias"},
        // W3 — Stronghold of the Faithful
        {"Escort Glenna to the stronghold's courtyard.",                   "Escort"},
        {"Defeat McLeod and breach the stronghold.",                       ""},
        {"Destroy the Keep Construct.",                                    "Keep Construct"},
        {"Traverse the Twisted Castle.",                                   "Twisted Castle"},
        {"Defeat Xera.",                                                   "Xera"},
        // W4 — Bastion of the Penitent
        {"Defeat Cairn the Indomitable.",                                  "Cairn"},
        {"Defeat the Mursaat Overseer.",                                   "Mursaat"},
        {"Defeat Samarog.",                                                "Samarog"},
        {"Free the prisoner from his bonds.",                              "Deimos"},
        // W5 — Hall of Chains
        {"Defeat the Soulless Horror.",                                    "Soulless Horror"},
        {"Traverse the River of Souls.",                                   "River"},
        {"Restore the Statue of Ice.",                                     "Broken King"},
        {"Restore the Statue of Death and Resurrection.",                  "Eater"},
        {"Restore the Statue of Darkness.",                                "Eyes"},
        {"Defeat Dhuum.",                                                  "Dhuum"},
        // W6 — Mythwright Gambit
        {"Destroy the conjured amalgamate.",                               "Conjured Amalgamate"},
        {"Make your way through the sorting and appraisal rooms.",         "Sorting"},
        {"Defeat the twin largos.",                                        "Twin Largos"},
        {"Clear a path through Qadim's minions to the Mythwright Cauldron.", "Qadim's Minions"},
        {"Defeat Qadim.",                                                  "Qadim"},
        // W7 — Key of Ahdashim
        {"Get Glenna and the key to Ahdashim's front gate.",               "Gate"},
        {"Defeat Cardinal Adina.",                                         "Adina"},
        {"Defeat Cardinal Sabir.",                                         "Sabir"},
        {"Defeat Qadim the Peerless.",                                     "Qadim the Peerless"},
        // W8 — Mount Balrior
        {"Cleanse the camp of titanspawn.",                                "Camp"},
        {"Defeat the sentient conduit.",                                   "Conduit"},
        {"Defeat the blighted beast and his empowering allies.",           ""},
        {"Defeat Decima.",                                                 "Decima"},
        {"Defeat Greer.",                                                  "Greer"},
        {"Defeat Ura.",                                                    "Ura"},
    };
    auto it = nameMap.find(text);
    if (it != nameMap.end()) return it->second;
    return text; // fallback to original
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

// ============= Banner-row helpers (strikes / raid wings) =============

#include "EmbeddedBanners.h"

// Look up the embedded banner JPEG for a key (e.g. "wing:9128", "strike:Cold War")
// and return a loaded Nexus texture, or nullptr if none.
static Texture_t* GetBannerTexture(const std::string& key) {
    unsigned int len = 0;
    const unsigned char* data = AlterEgo::EmbeddedBanners::FindBanner(key, len);
    if (!data || len == 0) return nullptr;

    std::string texId = "AE_BNR_" + key;
    Texture_t* tex = APIDefs->Textures_Get(texId.c_str());
    if (!tex || !tex->Resource) {
        tex = APIDefs->Textures_GetOrCreateFromMemory(
            texId.c_str(), const_cast<unsigned char*>(data), (uint64_t)len);
    }
    return (tex && tex->Resource) ? tex : nullptr;
}

struct ThemeGrad { ImU32 a, b; };

static ThemeGrad WingTheme(uint32_t wingId) {
    switch (wingId) {
        case 9128: return { IM_COL32(26, 58, 94, 255),  IM_COL32(74, 139, 201, 255) }; // W1 Spirit Vale (blue)
        case 9147: return { IM_COL32(74, 26, 26, 255),  IM_COL32(160, 60, 50, 255)  }; // W2 Salvation Pass (red)
        case 9182: return { IM_COL32(20, 60, 32, 255),  IM_COL32(80, 150, 80, 255)  }; // W3 Stronghold (green)
        case 9144: return { IM_COL32(58, 26, 78, 255),  IM_COL32(107, 42, 143, 255) }; // W4 Bastion (purple)
        case 9111: return { IM_COL32(28, 24, 38, 255),  IM_COL32(90, 80, 130, 255)  }; // W5 Hall of Chains (dark slate)
        case 9120: return { IM_COL32(78, 38, 14, 255),  IM_COL32(200, 110, 30, 255) }; // W6 Mythwright (gold/fire)
        case 9156: return { IM_COL32(74, 42, 20, 255),  IM_COL32(200, 154, 74, 255) }; // W7 Key of Ahdashim (tan)
        case 9181: return { IM_COL32(56, 28, 16, 255),  IM_COL32(150, 70, 40, 255)  }; // W8 Mount Balrior
    }
    return { IM_COL32(28, 26, 32, 255), IM_COL32(60, 56, 70, 255) };
}

static ThemeGrad FractalTierTheme(const std::string& tier) {
    if (tier == "T1")  return { IM_COL32(30, 70, 36, 255),   IM_COL32(110, 200, 122, 255) }; // green
    if (tier == "T2")  return { IM_COL32(26, 60, 110, 255),  IM_COL32(110, 192, 224, 255) }; // blue
    if (tier == "T3")  return { IM_COL32(80, 50, 14, 255),   IM_COL32(216, 161, 74, 255)  }; // orange
    if (tier == "T4")  return { IM_COL32(70, 22, 22, 255),   IM_COL32(217, 106, 106, 255) }; // red
    if (tier == "Rec") return { IM_COL32(58, 32, 78, 255),   IM_COL32(160, 110, 200, 255) }; // purple/gold-ish
    return { IM_COL32(40, 36, 28, 255), IM_COL32(120, 100, 60, 255) };
}

// Map a bounty boss name to its raid wing id (for banner background + theme).
// Returns 0 if unknown.
static const char* BountyWingFullName(uint32_t wingId) {
    switch (wingId) {
        case 9128: return "Spirit Vale";
        case 9147: return "Salvation Pass";
        case 9182: return "Stronghold of the Faithful";
        case 9144: return "Bastion of the Penitent";
        case 9111: return "Hall of Chains";
        case 9120: return "Mythwright Gambit";
        case 9156: return "The Key of Ahdashim";
        case 9181: return "Mount Balrior";
    }
    return "";
}

static const char* BountyWingShortName(uint32_t wingId) {
    switch (wingId) {
        case 9128: return "W1";
        case 9147: return "W2";
        case 9182: return "W3";
        case 9144: return "W4";
        case 9111: return "W5";
        case 9120: return "W6";
        case 9156: return "W7";
        case 9181: return "W8";
    }
    return "";
}

// Map a bounty name to a strike banner key (e.g. "strike:Cold War").
// Returns empty if no match. Handles bounty names that may be boss names
// rather than the strike's mission name.
static std::string BountyToStrikeName(const std::string& name) {
    static const std::unordered_map<std::string, std::string> map = {
        // Strike mission name (canonical) → banner key
        {"Shiverpeaks Pass",                          "Shiverpeaks Pass"},
        {"Voice of the Fallen and Claw of the Fallen","Voice of the Fallen and Claw of the Fallen"},
        {"Fraenir of Jormag",                         "Fraenir of Jormag"},
        {"Boneskinner",                               "Boneskinner"},
        {"Whisper of Jormag",                         "Whisper of Jormag"},
        {"Forging Steel",                             "Forging Steel"},
        {"Cold War",                                  "Cold War"},
        {"Aetherblade Hideout",                       "Aetherblade Hideout"},
        {"Xunlai Jade Junkyard",                      "Xunlai Jade Junkyard"},
        {"Kaineng Overlook",                          "Kaineng Overlook"},
        {"Harvest Temple",                            "Harvest Temple"},
        {"Old Lion's Court",                          "Old Lion's Court"},
        {"Cosmic Observatory",                        "Cosmic Observatory"},
        {"Temple of Febe",                            "Temple of Febe"},
        {"Guardian's Glade",                          "Guardian's Glade"},
        // Common boss-name aliases that resolve to the same banner
        {"Voice and Claw",                            "Voice of the Fallen and Claw of the Fallen"},
        {"Voice and Claw of the Fallen",              "Voice of the Fallen and Claw of the Fallen"},
        {"Fraenir",                                   "Fraenir of Jormag"},
        {"Whisper",                                   "Whisper of Jormag"},
        {"Mai Trin",                                  "Aetherblade Hideout"},
        {"Ankka",                                     "Xunlai Jade Junkyard"},
        {"Minister Li",                               "Kaineng Overlook"},
        {"The Dragonvoid",                            "Harvest Temple"},
        {"Dragonvoid",                                "Harvest Temple"},
        {"Dagda",                                     "Cosmic Observatory"},
        {"Cerus",                                     "Temple of Febe"},
        {"Eparch",                                    "Guardian's Glade"},
    };
    auto it = map.find(name);
    return (it != map.end()) ? it->second : std::string();
}

static uint32_t BountyBossToWing(const std::string& bossName) {
    static const std::unordered_map<std::string, uint32_t> map = {
        // W1 Spirit Vale
        {"Vale Guardian", 9128}, {"Gorseval", 9128}, {"Gorseval the Multifarious", 9128},
        {"Sabetha", 9128}, {"Sabetha the Saboteur", 9128},
        // W2 Salvation Pass
        {"Slothasor", 9147}, {"Bandit Trio", 9147}, {"Matthias", 9147}, {"Matthias Gabrel", 9147},
        // W3 Stronghold of the Faithful
        {"Keep Construct", 9182}, {"Xera", 9182},
        // W4 Bastion of the Penitent
        {"Cairn", 9144}, {"Cairn the Indomitable", 9144},
        {"Mursaat Overseer", 9144}, {"Samarog", 9144}, {"Deimos", 9144},
        // W5 Hall of Chains
        {"Soulless Horror", 9111}, {"Dhuum", 9111},
        // W6 Mythwright Gambit
        {"Conjured Amalgamate", 9120}, {"Twin Largos", 9120}, {"Largos", 9120}, {"Qadim", 9120},
        // W7 Key of Ahdashim
        {"Cardinal Adina", 9156}, {"Adina", 9156},
        {"Cardinal Sabir", 9156}, {"Sabir", 9156},
        {"Qadim the Peerless", 9156},
        // W8 Mount Balrior
        {"Greer", 9181}, {"Decima", 9181}, {"Ura", 9181},
    };
    auto it = map.find(bossName);
    return (it != map.end()) ? it->second : 0;
}

// Per-wiki: zone/map a strike mission is located in.
static const char* StrikeZone(const std::string& name) {
    static const std::unordered_map<std::string, std::string> map = {
        {"Shiverpeaks Pass",                          "Lornar's Pass"},
        {"Voice of the Fallen and Claw of the Fallen","Bjora Marches"},
        {"Fraenir of Jormag",                         "Bjora Marches"},
        {"Boneskinner",                               "Bjora Marches"},
        {"Whisper of Jormag",                         "Bjora Marches"},
        {"Forging Steel",                             "Drizzlewood Coast"},
        {"Cold War",                                  "Drizzlewood Coast"},
        {"Aetherblade Hideout",                       "Seitung Province"},
        {"Xunlai Jade Junkyard",                      "Echovald Wilds"},
        {"Kaineng Overlook",                          "New Kaineng City"},
        {"Harvest Temple",                            "Dragon's End"},
        {"Old Lion's Court",                          "Lion's Arch"},
        {"Cosmic Observatory",                        "Skywatch Archipelago"},
        {"Temple of Febe",                            "Inner Nayos"},
        {"Guardian's Glade",                          "Janthir Syntri"},
    };
    auto it = map.find(name);
    return (it != map.end()) ? it->second.c_str() : "";
}

static ThemeGrad StrikeTheme(const std::string& name) {
    // Hash → pick from a curated palette of 8 themes so colours stay stable per strike
    static const ThemeGrad palette[] = {
        { IM_COL32(26, 78, 110, 255), IM_COL32(90, 203, 230, 255) }, // ice blue
        { IM_COL32(110, 58, 20, 255), IM_COL32(255, 155, 58, 255) }, // ember
        { IM_COL32(78, 26, 26, 255),  IM_COL32(200, 74, 94, 255)  }, // crimson
        { IM_COL32(74, 74, 110, 255), IM_COL32(26, 26, 62, 255)   }, // void
        { IM_COL32(110, 90, 30, 255), IM_COL32(228, 196, 122, 255)}, // gold dust
        { IM_COL32(20, 70, 36, 255),  IM_COL32(100, 180, 100, 255)}, // verdant
        { IM_COL32(60, 30, 80, 255),  IM_COL32(160, 80, 200, 255) }, // amethyst
        { IM_COL32(20, 30, 50, 255),  IM_COL32(80, 110, 180, 255) }, // dusk
    };
    size_t h = std::hash<std::string>{}(name) % (sizeof(palette) / sizeof(palette[0]));
    return palette[h];
}

// Render a single banner row (strike / generic clear). If widthOverride > 0,
// use that width instead of the full available width (used for multi-column layouts).
// If bannerKey resolves to an embedded loading-screen JPEG, that image is used
// as the background; otherwise the gradient theme is used.
static void RenderClearBanner(const std::string& title, const std::string& subtitle,
                              bool fetched, bool done, ThemeGrad theme,
                              const char* tooltip = nullptr,
                              float widthOverride = 0.0f,
                              const std::string& bannerKey = "") {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cmin = ImGui::GetCursorScreenPos();
    float w = widthOverride > 0.0f ? widthOverride : ImGui::GetContentRegionAvail().x;
    float h = 46.0f;
    ImVec2 cmax(cmin.x + w, cmin.y + h);

    // Background: embedded loading screen if available, else gradient
    Texture_t* bnrTex = bannerKey.empty() ? nullptr : GetBannerTexture(bannerKey);
    if (bnrTex && bnrTex->Resource) {
        dl->AddImage(bnrTex->Resource, cmin, cmax,
            ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
    } else {
        dl->AddRectFilledMultiColor(cmin, cmax,
            theme.a, theme.b,
            IM_COL32(theme.b & 0xFF, (theme.b >> 8) & 0xFF, (theme.b >> 16) & 0xFF, 255),
            theme.a);
    }

    // Dark scrim — solid on the left, fading right
    ImU32 scrimL = IM_COL32(14, 14, 18, 235);
    ImU32 scrimR = IM_COL32(14, 14, 18, 26);
    dl->AddRectFilledMultiColor(cmin, cmax, scrimL, scrimR, scrimR, scrimL);

    // Border
    dl->AddRect(cmin, cmax, IM_COL32(0, 0, 0, 200), 4.0f, 0, 1.0f);

    // Hit test (whole banner)
    ImGui::InvisibleButton("##banner", ImVec2(w, h));
    bool hovered = ImGui::IsItemHovered();
    if (hovered) {
        dl->AddRect(cmin, cmax,
            IM_COL32(197, 161, 85, 130), 4.0f, 0, 1.0f);
        if (tooltip) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(tooltip);
            ImGui::EndTooltip();
        }
    }

    // Status circle on the left
    float cx = cmin.x + 18.0f;
    float cy = cmin.y + h * 0.5f;
    float r = 10.0f;
    ImU32 ringCol, fillCol, checkCol;
    if (!fetched) {
        ringCol = IM_COL32(120, 110, 90, 220);
        fillCol = IM_COL32(0, 0, 0, 130);
        checkCol = IM_COL32(160, 150, 120, 200);
    } else if (done) {
        ringCol = IM_COL32(111, 204, 122, 255);
        fillCol = IM_COL32(50, 110, 60, 130);
        checkCol = IM_COL32(180, 240, 180, 255);
    } else {
        ringCol = IM_COL32(160, 150, 120, 200);
        fillCol = IM_COL32(0, 0, 0, 130);
        checkCol = IM_COL32(160, 150, 120, 200);
    }
    dl->AddCircleFilled(ImVec2(cx, cy), r, fillCol);
    dl->AddCircle(ImVec2(cx, cy), r, ringCol, 24, 1.6f);
    if (fetched && done) {
        // Draw checkmark
        dl->AddLine(ImVec2(cx - 4, cy + 0),  ImVec2(cx - 1, cy + 3),  checkCol, 2.0f);
        dl->AddLine(ImVec2(cx - 1, cy + 3),  ImVec2(cx + 5, cy - 4),  checkCol, 2.0f);
    } else if (!fetched) {
        // ?
        ImVec2 ts = ImGui::CalcTextSize("?");
        dl->AddText(ImVec2(cx - ts.x * 0.5f, cy - ts.y * 0.5f), checkCol, "?");
    }

    // Title (top) + subtitle (bottom)
    float textX = cmin.x + 36.0f;
    float titleY = cmin.y + 6.0f;
    float subY = cmin.y + h - 6.0f - ImGui::GetTextLineHeight();
    ImU32 titleCol = (fetched && done) ? IM_COL32(240, 240, 230, 255) : IM_COL32(220, 215, 200, 255);
    ImU32 subCol = IM_COL32(160, 152, 130, 230);
    dl->AddText(ImVec2(textX, titleY), titleCol, title.c_str());
    if (!subtitle.empty())
        dl->AddText(ImVec2(textX, subY), subCol, subtitle.c_str());

    // Spacing for next item
    ImGui::Dummy(ImVec2(0, 2));
}

struct WingEncChip { std::string label; std::string fullName; bool done = false; };

// Compact single-row tier banner with inline chips — for daily fractals.
// Layout: [accent] [TIER] [title]   [chip][chip][chip]   X/Y
static void RenderTierRow(const char* tierLabel,
                          const std::string& title,
                          int doneCount, int totalCount,
                          bool fetched,
                          ThemeGrad theme,
                          ImU32 accentColor,
                          const std::vector<WingEncChip>& chips) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cmin = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float h = 46.0f;
    ImVec2 cmax(cmin.x + w, cmin.y + h);

    // Gradient background
    dl->AddRectFilledMultiColor(cmin, cmax,
        theme.a, theme.b,
        IM_COL32(theme.b & 0xFF, (theme.b >> 8) & 0xFF, (theme.b >> 16) & 0xFF, 255),
        theme.a);

    // Left-biased scrim
    ImU32 scrimL = IM_COL32(14, 14, 18, 235);
    ImU32 scrimR = IM_COL32(14, 14, 18, 80);
    dl->AddRectFilledMultiColor(cmin, cmax, scrimL, scrimR, scrimR, scrimL);

    dl->AddRect(cmin, cmax, IM_COL32(0, 0, 0, 200), 4.0f, 0, 1.0f);

    // Left accent strip
    dl->AddRectFilled(ImVec2(cmin.x, cmin.y + 3), ImVec2(cmin.x + 3, cmax.y - 3),
        accentColor);

    float padX = 10.0f;
    float midY = cmin.y + h * 0.5f;

    // Tier label (left, tier-coloured)
    float tierX = cmin.x + padX;
    ImVec2 tierSz = ImGui::CalcTextSize(tierLabel);
    dl->AddText(ImVec2(tierX, midY - tierSz.y * 0.5f), accentColor, tierLabel);

    // Title
    float titleX = tierX + tierSz.x + 10.0f;
    ImU32 titleCol = IM_COL32(230, 225, 210, 255);
    ImVec2 titleSz = ImGui::CalcTextSize(title.c_str());
    dl->AddText(ImVec2(titleX, midY - titleSz.y * 0.5f), titleCol, title.c_str());

    // Progress (right)
    char prog[32];
    snprintf(prog, sizeof(prog), "%d / %d", doneCount, totalCount);
    ImVec2 progSz = ImGui::CalcTextSize(prog);
    ImU32 progCol = (fetched && doneCount == totalCount && totalCount > 0)
        ? IM_COL32(180, 240, 180, 255)
        : IM_COL32(227, 196, 122, 255);
    float progX = cmax.x - padX - progSz.x;
    dl->AddText(ImVec2(progX, midY - progSz.y * 0.5f), progCol, prog);

    // Hit-test the whole row first (so chips can override with their own)
    ImGui::InvisibleButton("##tierrow", ImVec2(w, h));

    // Chips area between title and progress
    float chipsStartX = titleX + titleSz.x + 12.0f;
    float chipsEndX = progX - 10.0f;
    float chipsW = chipsEndX - chipsStartX;
    float chipH = 18.0f;
    float chipY = midY - chipH * 0.5f;
    if (!chips.empty() && chipsW > 40.0f) {
        float gap = 4.0f;
        float chipW = (chipsW - gap * (chips.size() - 1)) / (float)chips.size();
        for (size_t i = 0; i < chips.size(); i++) {
            ImVec2 chmin(chipsStartX + (chipW + gap) * i, chipY);
            ImVec2 chmax(chmin.x + chipW, chmin.y + chipH);

            dl->AddRectFilled(chmin, chmax, IM_COL32(0, 0, 0, 175), 3.0f);
            ImU32 cborder = chips[i].done
                ? IM_COL32(111, 204, 122, 200)
                : IM_COL32(160, 145, 110, 110);
            dl->AddRect(chmin, chmax, cborder, 3.0f, 0, 1.0f);

            float dotR = 3.5f;
            float dotX = chmin.x + 7.0f;
            float dotY = chmin.y + chipH * 0.5f;
            if (chips[i].done)
                dl->AddCircleFilled(ImVec2(dotX, dotY), dotR, IM_COL32(111, 204, 122, 255));
            else
                dl->AddCircle(ImVec2(dotX, dotY), dotR, IM_COL32(160, 145, 110, 220), 16, 1.4f);

            ImU32 lblCol = chips[i].done
                ? IM_COL32(240, 235, 215, 255)
                : IM_COL32(170, 160, 135, 220);
            float lblX = dotX + dotR + 5.0f;
            ImVec2 lblSz = ImGui::CalcTextSize(chips[i].label.c_str());
            float lblY = chmin.y + (chipH - lblSz.y) * 0.5f;
            dl->PushClipRect(chmin, chmax, true);
            dl->AddText(ImVec2(lblX, lblY), lblCol, chips[i].label.c_str());
            dl->PopClipRect();

            // Per-chip hover (overlay on top of the row hit area)
            ImGui::SetCursorScreenPos(chmin);
            ImGui::PushID((int)i);
            ImGui::InvisibleButton("##chip", ImVec2(chipW, chipH));
            if (ImGui::IsItemHovered() && !chips[i].fullName.empty()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(chips[i].fullName.c_str());
                ImGui::EndTooltip();
                dl->AddRect(chmin, chmax, IM_COL32(197, 161, 85, 220), 3.0f, 0, 1.0f);
            }
            ImGui::PopID();
        }
    }

    ImGui::SetCursorScreenPos(ImVec2(cmin.x, cmin.y + h));
    ImGui::Dummy(ImVec2(0, 2));
}


// ============================================================
// Weekly raid wing rotations (Emboldened / Call of the Mists)
// ============================================================
//
// Both rotations advance every Monday at 08:30 UTC through the 8 wings
// in release order. Call of the Mists sits one slot ahead of Emboldened.
// Anchor: Monday 2026-05-18 08:30 UTC → Salvation Pass is emboldened
//                                       Stronghold has Call of the Mists.
// Verified against wiki.guildwars2.com/wiki/Emboldened and Call_of_the_Mists.

static constexpr int64_t kRotationAnchorUnix = 1779093000; // 2026-05-18 08:30 UTC
static constexpr int     kRotationAnchorEmboldenedIndex = 1; // W2 Salvation Pass
static constexpr int64_t kWeekSeconds = 7 * 24 * 3600;

// Wing IDs in rotation order (W1..W8).
static uint32_t WingIdByRotationIndex(int idx) {
    static const uint32_t kWingByIdx[8] = {
        9128, // W1 Spirit Vale
        9147, // W2 Salvation Pass
        9182, // W3 Stronghold of the Faithful
        9144, // W4 Bastion of the Penitent
        9111, // W5 Hall of Chains
        9120, // W6 Mythwright Gambit
        9156, // W7 The Key of Ahdashim
        9181, // W8 Mount Balrior
    };
    if (idx < 0 || idx >= 8) return 0;
    return kWingByIdx[idx];
}

static int CurrentEmboldenedRotationIndex() {
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t diff = now - kRotationAnchorUnix;
    int64_t weeks = diff / kWeekSeconds;
    if (diff < 0 && (diff % kWeekSeconds) != 0) weeks -= 1; // floor toward -inf
    int idx = (int)(((weeks + kRotationAnchorEmboldenedIndex) % 8 + 8) % 8);
    return idx;
}

static uint32_t CurrentEmboldenedWingId() {
    return WingIdByRotationIndex(CurrentEmboldenedRotationIndex());
}

static uint32_t CurrentCallOfTheMistsWingId() {
    return WingIdByRotationIndex((CurrentEmboldenedRotationIndex() + 1) % 8);
}


static void RenderWingBanner(const std::string& wingShortName,    // e.g. "W4"
                             const std::string& wingFullName,     // e.g. "Bastion of the Penitent"
                             int doneCount, int totalCount,
                             bool fetched,
                             ThemeGrad theme,
                             const std::vector<WingEncChip>& chips,
                             const std::string& bannerKey = "",
                             bool isEmboldened = false,
                             bool isCallOfTheMists = false) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cmin = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float h = 78.0f;
    ImVec2 cmax(cmin.x + w, cmin.y + h);

    // Background: embedded loading screen if available, else gradient
    Texture_t* bnrTex = bannerKey.empty() ? nullptr : GetBannerTexture(bannerKey);
    if (bnrTex && bnrTex->Resource) {
        dl->AddImage(bnrTex->Resource, cmin, cmax,
            ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
    } else {
        dl->AddRectFilledMultiColor(cmin, cmax,
            theme.a, theme.b,
            IM_COL32(theme.b & 0xFF, (theme.b >> 8) & 0xFF, (theme.b >> 16) & 0xFF, 255),
            theme.a);
    }

    // Vertical scrim — fade top to bottom so the lower chip area is readable
    ImU32 topScrim = IM_COL32(14, 14, 18, 100);
    ImU32 botScrim = IM_COL32(14, 14, 18, 235);
    dl->AddRectFilledMultiColor(cmin, cmax, topScrim, topScrim, botScrim, botScrim);
    // Slight left bias too
    ImU32 leftScrim = IM_COL32(14, 14, 18, 130);
    ImU32 rightScrim = IM_COL32(14, 14, 18, 0);
    dl->AddRectFilledMultiColor(cmin, cmax, leftScrim, rightScrim, rightScrim, leftScrim);

    dl->AddRect(cmin, cmax, IM_COL32(0, 0, 0, 200), 5.0f, 0, 1.0f);

    // Top row text
    float padX = 10.0f;
    float topY = cmin.y + 8.0f;
    // Roman
    dl->AddText(ImVec2(cmin.x + padX, topY),
        IM_COL32(227, 196, 122, 255), wingShortName.c_str());
    // Name
    ImVec2 wsSz = ImGui::CalcTextSize(wingShortName.c_str());
    dl->AddText(ImVec2(cmin.x + padX + wsSz.x + 10.0f, topY),
        IM_COL32(240, 235, 220, 255), wingFullName.c_str());
    ImVec2 nameSz = ImGui::CalcTextSize(wingFullName.c_str());
    // Progress fraction (right-aligned)
    char prog[32];
    snprintf(prog, sizeof(prog), "%d / %d", doneCount, totalCount);
    ImVec2 pSz = ImGui::CalcTextSize(prog);
    ImU32 progCol = (fetched && doneCount == totalCount && totalCount > 0)
        ? IM_COL32(180, 240, 180, 255)
        : IM_COL32(227, 196, 122, 255);
    dl->AddText(ImVec2(cmax.x - padX - pSz.x, topY), progCol, prog);

    // Rotation badges (Emboldened / Call of the Mists) — drawn between the
    // wing name and the progress fraction, with a wiki icon + label.
    if (isEmboldened || isCallOfTheMists) {
        float badgeStartX = cmin.x + padX + wsSz.x + 10.0f + nameSz.x + 12.0f;
        float rightLimit = cmax.x - padX - pSz.x - 10.0f;
        float curX = badgeStartX;
        float iconSz = 16.0f;
        float padIn = 5.0f;
        float badgeH = iconSz + 4.0f;
        float badgeY = topY - 2.0f;

        auto drawBadge = [&](const char* iconKey, const char* label,
                             const char* tip, ImU32 borderCol) {
            ImVec2 lblSz = ImGui::CalcTextSize(label);
            float badgeW = padIn + iconSz + 5.0f + lblSz.x + padIn;
            if (curX + badgeW > rightLimit) return;
            ImVec2 bmin(curX, badgeY);
            ImVec2 bmax(bmin.x + badgeW, bmin.y + badgeH);
            dl->AddRectFilled(bmin, bmax, IM_COL32(0, 0, 0, 170), 3.0f);
            dl->AddRect(bmin, bmax, borderCol, 3.0f, 0, 1.0f);
            Texture_t* tex = Skinventory::WikiImage::GetCurrencyIcon(iconKey);
            if (tex && tex->Resource) {
                dl->AddImage(tex->Resource,
                    ImVec2(bmin.x + padIn, bmin.y + 2.0f),
                    ImVec2(bmin.x + padIn + iconSz, bmin.y + 2.0f + iconSz));
            }
            float lblX = bmin.x + padIn + iconSz + 5.0f;
            float lblY = bmin.y + (badgeH - lblSz.y) * 0.5f;
            dl->AddText(ImVec2(lblX, lblY),
                IM_COL32(240, 235, 220, 255), label);
            ImVec2 saveCursor = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos(bmin);
            ImGui::PushID(iconKey);
            ImGui::InvisibleButton("##rb", ImVec2(badgeW, badgeH));
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(tip);
                ImGui::EndTooltip();
                dl->AddRect(bmin, bmax,
                    IM_COL32(255, 220, 130, 230), 3.0f, 0, 1.4f);
            }
            ImGui::PopID();
            ImGui::SetCursorScreenPos(saveCursor);
            curX += badgeW + 6.0f;
        };

        if (isCallOfTheMists) {
            drawBadge("Call_of_the_Mists", "Call of the Mists",
                "Call of the Mists — boss gold and XP are doubled this week.",
                IM_COL32(227, 196, 122, 220));
        }
        if (isEmboldened) {
            drawBadge("Emboldened", "Emboldened",
                "Emboldened Mode — stacking buff for easier clears this week.",
                IM_COL32(120, 180, 240, 220));
        }
    }

    // Encounter chips along the bottom
    float chipsY = cmin.y + h - 24.0f;
    float chipsX = cmin.x + padX;
    float chipsW = w - padX * 2;
    float chipH = 18.0f;
    if (!chips.empty()) {
        float gap = 4.0f;
        float chipW = (chipsW - gap * (chips.size() - 1)) / (float)chips.size();
        for (size_t i = 0; i < chips.size(); i++) {
            ImVec2 chmin(chipsX + (chipW + gap) * i, chipsY);
            ImVec2 chmax(chmin.x + chipW, chmin.y + chipH);

            // Chip bg
            dl->AddRectFilled(chmin, chmax, IM_COL32(0, 0, 0, 165), 3.0f);
            ImU32 cborder = chips[i].done
                ? IM_COL32(111, 204, 122, 200)
                : IM_COL32(160, 145, 110, 110);
            dl->AddRect(chmin, chmax, cborder, 3.0f, 0, 1.0f);

            // Dot
            float dotR = 4.0f;
            float dotX = chmin.x + 7.0f;
            float dotY = chmin.y + chipH * 0.5f;
            if (chips[i].done) {
                dl->AddCircleFilled(ImVec2(dotX, dotY), dotR,
                    IM_COL32(111, 204, 122, 255));
            } else {
                dl->AddCircle(ImVec2(dotX, dotY), dotR,
                    IM_COL32(160, 145, 110, 220), 16, 1.4f);
            }

            // Label
            ImU32 lblCol = chips[i].done
                ? IM_COL32(240, 235, 215, 255)
                : IM_COL32(170, 160, 135, 220);
            float lblX = dotX + dotR + 5.0f;
            ImVec2 lblSz = ImGui::CalcTextSize(chips[i].label.c_str());
            float lblY = chmin.y + (chipH - lblSz.y) * 0.5f;
            // Clip text to chip width
            float maxLblW = chipW - (lblX - chmin.x) - 4.0f;
            dl->PushClipRect(chmin, chmax, true);
            (void)maxLblW;
            dl->AddText(ImVec2(lblX, lblY), lblCol, chips[i].label.c_str());
            dl->PopClipRect();

            // Hover area + tooltip with full encounter name
            ImGui::SetCursorScreenPos(chmin);
            ImGui::PushID((int)i);
            ImGui::InvisibleButton("##chip", ImVec2(chipW, chipH));
            if (ImGui::IsItemHovered() && !chips[i].fullName.empty()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(chips[i].fullName.c_str());
                ImGui::EndTooltip();
                // Brighten border on hover
                dl->AddRect(chmin, chmax, IM_COL32(197, 161, 85, 220), 3.0f, 0, 1.0f);
            }
            ImGui::PopID();
        }
    }

    // Final cursor advance for the whole banner
    ImGui::SetCursorScreenPos(ImVec2(cmin.x, cmax.y + 4.0f));
}

// Get wing short name "W1"-"W8" from wing ID, fallback empty string.
static const char* WingShortName(uint32_t wingId) {
    switch (wingId) {
        case 9128: return "W1";
        case 9147: return "W2";
        case 9182: return "W3";
        case 9144: return "W4";
        case 9111: return "W5";
        case 9120: return "W6";
        case 9156: return "W7";
        case 9181: return "W8";
    }
    return "";
}

// Get clean wing name with "Weekly " stripped
static std::string CleanWingName(const std::string& raw) {
    if (raw.rfind("Weekly ", 0) == 0) return raw.substr(7);
    return raw;
}

// =========================================================================
// Clears - Vault tab rendering
// =========================================================================

// Per-track accent colour for objective rows
static ImU32 VaultTrackColor(const std::string& track) {
    if (track == "PvP") return IM_COL32(204, 84, 84, 255);   // red
    if (track == "WvW") return IM_COL32(84, 135, 204, 255);  // blue
    return IM_COL32(84, 186, 84, 255);                       // PvE green
}

// Slim two-column row for a single vault objective.
static void RenderVaultObjectiveRow(const VaultObjective& obj, float width) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cmin = ImGui::GetCursorScreenPos();
    float h = 26.0f;
    ImVec2 cmax(cmin.x + width, cmin.y + h);

    bool done = obj.isDone();
    bool claimed = obj.claimed;

    // Subtle background (slightly lifted for unclaimed)
    ImU32 bg = claimed ? IM_COL32(18, 20, 24, 200)
                       : IM_COL32(26, 28, 34, 220);
    dl->AddRectFilled(cmin, cmax, bg, 3.0f);
    dl->AddRect(cmin, cmax, IM_COL32(0, 0, 0, 160), 3.0f, 0, 1.0f);

    // Left accent strip — track colour, dimmed when claimed
    ImU32 accent = VaultTrackColor(obj.track);
    if (claimed) accent = (accent & 0x00FFFFFF) | (110 << 24);
    dl->AddRectFilled(ImVec2(cmin.x, cmin.y + 2),
                      ImVec2(cmin.x + 3, cmax.y - 2), accent);

    float padX = 8.0f;
    float midY = cmin.y + h * 0.5f;

    // Track tag (compact)
    float tagX = cmin.x + padX;
    if (!obj.track.empty()) {
        ImU32 tagCol = claimed ? IM_COL32(140, 130, 110, 200)
                               : VaultTrackColor(obj.track);
        ImVec2 tagSz = ImGui::CalcTextSize(obj.track.c_str());
        dl->AddText(ImVec2(tagX, midY - tagSz.y * 0.5f), tagCol, obj.track.c_str());
        tagX += tagSz.x + 8.0f;
    }

    // Acclaim pill (right-aligned)
    char pill[16];
    snprintf(pill, sizeof(pill), "+%d", obj.acclaim);
    ImVec2 pillSz = ImGui::CalcTextSize(pill);
    float pillPadX = 5.0f, pillPadY = 2.0f;
    float pillW = pillSz.x + pillPadX * 2;
    float pillH = pillSz.y + pillPadY * 2;
    ImVec2 pillMin(cmax.x - padX - pillW, midY - pillH * 0.5f);
    ImVec2 pillMax(pillMin.x + pillW, pillMin.y + pillH);

    ImU32 pillBg, pillBorder, pillText;
    if (claimed) {
        pillBg     = IM_COL32(40, 60, 40, 180);
        pillBorder = IM_COL32(80, 140, 90, 160);
        pillText   = IM_COL32(140, 200, 150, 220);
    } else if (done) {
        // earned but unclaimed — the actionable state, brightest
        pillBg     = IM_COL32(80, 60, 18, 230);
        pillBorder = IM_COL32(232, 196, 122, 255);
        pillText   = IM_COL32(245, 220, 150, 255);
    } else {
        pillBg     = IM_COL32(32, 28, 18, 200);
        pillBorder = IM_COL32(140, 116, 60, 200);
        pillText   = IM_COL32(220, 192, 122, 230);
    }
    dl->AddRectFilled(pillMin, pillMax, pillBg, 3.0f);
    dl->AddRect(pillMin, pillMax, pillBorder, 3.0f, 0, 1.0f);
    dl->AddText(ImVec2(pillMin.x + pillPadX, pillMin.y + pillPadY), pillText, pill);

    // Title (clipped between tagX and pillMin.x)
    ImU32 titleCol;
    if (claimed)   titleCol = IM_COL32(130, 130, 120, 230);
    else if (done) titleCol = IM_COL32(200, 200, 190, 255);
    else           titleCol = IM_COL32(235, 230, 215, 255);

    ImVec2 titleSz = ImGui::CalcTextSize(obj.title.c_str());
    float titleX = tagX;
    float titleEndX = pillMin.x - 8.0f;

    // If there's room and the obj is in-progress, show "N/M" right before the pill
    char progBuf[24] = {0};
    bool showProg = !done && obj.progress_complete > 1;
    float progW = 0.0f;
    if (showProg) {
        snprintf(progBuf, sizeof(progBuf), "%d/%d", obj.progress_current, obj.progress_complete);
        progW = ImGui::CalcTextSize(progBuf).x + 6.0f;
    }
    float titleClipEnd = titleEndX - progW;

    dl->PushClipRect(ImVec2(titleX, cmin.y), ImVec2(titleClipEnd, cmax.y), true);
    dl->AddText(ImVec2(titleX, midY - titleSz.y * 0.5f), titleCol, obj.title.c_str());
    dl->PopClipRect();

    if (showProg) {
        ImVec2 pSz = ImGui::CalcTextSize(progBuf);
        dl->AddText(ImVec2(titleEndX - pSz.x, midY - pSz.y * 0.5f),
            IM_COL32(150, 142, 120, 230), progBuf);
    }

    // Hit test (whole row) — tooltip with full title if it was truncated
    ImGui::InvisibleButton("##vobj", ImVec2(width, h));
    if (ImGui::IsItemHovered()) {
        bool overflowed = (titleSz.x > (titleClipEnd - titleX));
        if (overflowed) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(obj.title.c_str());
            ImGui::EndTooltip();
        }
        dl->AddRect(cmin, cmax, IM_COL32(197, 161, 85, 130), 3.0f, 0, 1.0f);
    }
}

static void RenderVaultPeriodSection(const char* title, const char* resetLabel,
                                      ImVec4 color, const VaultPeriod& period)
{
    char suffix[64];
    snprintf(suffix, sizeof(suffix), "(%s)", resetLabel);
    RenderSectionHeader(title, color, suffix);

    if (period.objectives.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
        return;
    }

    // Inline summary: claimed count + total AA earned.
    // Meta bonus AA is added to the totals so users see the true cap
    // (e.g. daily 4/5 grants a meta bonus; weekly 6/8 grants a meta bonus).
    int claimedCount = 0, totalAA = 0, earnedAA = 0;
    for (const auto& o : period.objectives) {
        if (o.claimed) { claimedCount++; earnedAA += o.acclaim; }
        totalAA += o.acclaim;
    }
    totalAA += period.meta_reward_astral;
    if (period.meta_reward_claimed) earnedAA += period.meta_reward_astral;

    ImGui::Indent(4.0f);
    ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f),
        "%d / %d claimed   %d / %d AA",
        claimedCount, (int)period.objectives.size(),
        earnedAA, totalAA);

    // Thin meta progress bar (only if API gave us meta data)
    if (period.meta_progress_complete > 0) {
        float frac = std::min(1.0f,
            (float)period.meta_progress_current / (float)period.meta_progress_complete);
        ImVec4 barColor = period.meta_reward_claimed
            ? ImVec4(0.35f, 0.82f, 0.35f, 1.0f) : color;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.13f, 1.0f));
        ImGui::ProgressBar(frac, ImVec2(-4.0f, 3.0f), "");
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();

    // Two-column grid of objective rows
    float availW = ImGui::GetContentRegionAvail().x - 4.0f;
    float gap = 6.0f;
    float halfW = (availW - gap) * 0.5f;

    for (size_t i = 0; i < period.objectives.size(); i++) {
        const auto& obj = period.objectives[i];

        ImGui::PushID((int)i);
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        RenderVaultObjectiveRow(obj, halfW);
        ImGui::PopID();

        if (i % 2 == 0 && i + 1 < period.objectives.size()) {
            ImGui::SetCursorScreenPos(ImVec2(startPos.x + halfW + gap, startPos.y));
        } else {
            ImGui::Dummy(ImVec2(0, 2));
        }
    }

    ImGui::Unindent(4.0f);
}

static void RenderVaultTab(const std::string& dailyResetStr, const std::string& weeklyResetStr)
{
    // Consume retry (H&S was PENDING last call)
    if (g_VaultRetryPending.exchange(false) && !g_VaultFetching)
        FetchVaultData();

    // Account change: clear stale data and re-fetch
    if (g_VaultNeedRequery && !g_VaultFetching) {
        g_VaultNeedRequery = false;
        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_VaultDaily = {}; g_VaultWeekly = {}; g_VaultSpecial = {};
        }
        g_VaultFetched = false;
        FetchVaultData();
    }

    // Auto-fetch on first open (or after reset invalidation)
    if (!g_VaultFetched && !g_VaultFetching)
        FetchVaultData();

    // Refresh button + status
    bool fetching = g_VaultFetching;
    if (fetching) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    if (ImGui::SmallButton("Refresh##vault") && !fetching) {
        g_VaultFetched = false;
        FetchVaultData();
    }
    if (fetching) ImGui::PopStyleVar();
    ImGui::SameLine();
    if (fetching)
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Loading...");
    else if (!g_VaultFetched)
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No data — click Refresh");
    else
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Data loaded");

    ImGui::Separator();

    std::lock_guard<std::mutex> lock(g_ClearsMutex);

    RenderVaultPeriodSection("Daily Objectives", dailyResetStr.c_str(),
        ImVec4(0.61f, 0.43f, 0.92f, 1.0f), g_VaultDaily);

    ImGui::Spacing();
    ImGui::Separator();

    RenderVaultPeriodSection("Weekly Objectives", weeklyResetStr.c_str(),
        ImVec4(0.91f, 0.78f, 0.29f, 1.0f), g_VaultWeekly);

    if (!g_VaultSpecial.objectives.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        std::string seasonLabel = "seasonal";
        if (g_VaultSeasonEnd != std::chrono::system_clock::time_point{}) {
            auto sNow = std::chrono::system_clock::now();
            if (g_VaultSeasonEnd > sNow) {
                auto rem = std::chrono::duration_cast<std::chrono::seconds>(g_VaultSeasonEnd - sNow).count();
                int d = (int)(rem / 86400), h = (int)((rem % 86400) / 3600);
                char buf[32];
                snprintf(buf, sizeof(buf), "%dd %dh", d, h);
                seasonLabel = buf;
            } else {
                seasonLabel = "ended";
            }
        }
        RenderVaultPeriodSection("Special Objectives", seasonLabel.c_str(),
            ImVec4(0.9f, 0.70f, 0.30f, 1.0f), g_VaultSpecial);
    }
}

// =========================================================================
// Clears - Main render function
// =========================================================================

static void RenderClearsTabContent(const std::string& dailyResetStr, const std::string& weeklyResetStr)
{
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

    // ---- Daily Fractals ----
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", dailyResetStr.c_str());
        RenderSectionHeader("Daily Fractals", ImVec4(0.3f, 0.7f, 0.9f, 1.0f), suffix);
    }

    if (g_DailyFractals.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        ImGui::Indent(4.0f);
        struct TierInfo { const char* tier; const char* label; const char* fullName; ImU32 accent; };
        const TierInfo tierOrder[] = {
            {"T1",  "T1",  "Tier 1 Fractal", IM_COL32(111, 204, 122, 255)},
            {"T2",  "T2",  "Tier 2 Fractal", IM_COL32(110, 192, 224, 255)},
            {"T3",  "T3",  "Tier 3 Fractal", IM_COL32(216, 161, 74, 255)},
            {"T4",  "T4",  "Tier 4 Fractal", IM_COL32(217, 106, 106, 255)},
            {"Rec", "REC", "Recommended",    IM_COL32(197, 161, 85, 255)},
        };
        for (const auto& ti : tierOrder) {
            std::vector<const ClearEntry*> tierEntries;
            for (const auto& e : g_DailyFractals) {
                if (e.tier == ti.tier) tierEntries.push_back(&e);
            }
            if (tierEntries.empty()) continue;

            std::sort(tierEntries.begin(), tierEntries.end(),
                [](const ClearEntry* a, const ClearEntry* b) {
                    return ShortenFractalName(a->name) < ShortenFractalName(b->name);
                });

            std::vector<WingEncChip> chips;
            int doneCount = 0;
            for (const auto* e : tierEntries) {
                WingEncChip c;
                c.label = ShortenFractalName(e->name);
                c.fullName = e->name;
                c.done = e->done;
                chips.push_back(c);
                if (e->done) doneCount++;
            }

            ImGui::PushID(ti.tier);
            RenderTierRow(ti.label, ti.fullName,
                doneCount, (int)tierEntries.size(), g_ClearsFetched,
                FractalTierTheme(ti.tier), ti.accent, chips);
            ImGui::PopID();
        }
        // Uncategorized fractals (no tier) — show as plain rows
        for (const auto& e : g_DailyFractals) {
            if (!e.tier.empty()) continue;
            ImGui::Text("  ");
            ImGui::SameLine();
            RenderClearStatus(g_ClearsFetched, e.done, e.name.c_str());
        }
        ImGui::Unindent(4.0f);
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Daily Raid Bounties ----
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", dailyResetStr.c_str());
        RenderSectionHeader("Daily Raid Bounties", ImVec4(0.9f, 0.6f, 0.3f, 1.0f), suffix);
    }

    if (g_DailyBounties.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        ImGui::Indent(4.0f);
        float availW = ImGui::GetContentRegionAvail().x - 4.0f;
        float gapBnty = 6.0f;
        float halfWBnty = (availW - gapBnty) * 0.5f;
        for (size_t i = 0; i < g_DailyBounties.size(); i++) {
            const auto& e = g_DailyBounties[i];
            std::string display = e.name;
            if (display.find("Raid Bounty: ") == 0)
                display = display.substr(13);

            uint32_t wingId = BountyBossToWing(display);
            std::string strikeName = wingId ? std::string() : BountyToStrikeName(display);
            std::string subtitle;
            std::string bannerKey;
            ThemeGrad theme;
            if (wingId) {
                const char* wingShort = BountyWingShortName(wingId);
                const char* wingFull  = BountyWingFullName(wingId);
                if (wingShort && *wingShort) { subtitle = wingShort; }
                if (wingFull && *wingFull) {
                    if (!subtitle.empty()) subtitle += " \xC2\xB7 ";
                    subtitle += wingFull;
                }
                if (subtitle.empty()) subtitle = "Raid Bounty";
                bannerKey = "wing:" + std::to_string(wingId);
                theme = WingTheme(wingId);
            } else if (!strikeName.empty()) {
                subtitle = "Strike Mission";
                const char* zone = StrikeZone(strikeName);
                if (zone && *zone) { subtitle += " \xC2\xB7 "; subtitle += zone; }
                bannerKey = "strike:" + strikeName;
                theme = StrikeTheme(strikeName);
            } else {
                subtitle = "Raid Bounty";
                theme = ThemeGrad{ IM_COL32(60, 44, 20, 255), IM_COL32(200, 154, 74, 255) };
            }

            ImGui::PushID((int)i);
            ImVec2 startPos = ImGui::GetCursorScreenPos();
            RenderClearBanner(display, subtitle,
                g_ClearsFetched, e.done, theme,
                /*tooltip*/ nullptr, /*widthOverride*/ halfWBnty,
                bannerKey);
            ImGui::PopID();

            if (i % 2 == 0 && i + 1 < g_DailyBounties.size()) {
                ImGui::SetCursorScreenPos(ImVec2(startPos.x + halfWBnty + gapBnty, startPos.y));
            }
        }
        ImGui::Unindent(4.0f);
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Weekly Strikes ----
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", weeklyResetStr.c_str());
        RenderSectionHeader("Weekly Strikes", ImVec4(0.7f, 0.4f, 0.9f, 1.0f), suffix);
    }

    if (g_WeeklyStrikes.id == 0) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        ImGui::Indent(4.0f);
        // Two-column layout
        float availW = ImGui::GetContentRegionAvail().x - 4.0f;
        float gap = 6.0f;
        float halfW = (availW - gap) * 0.5f;

        for (size_t i = 0; i < g_WeeklyStrikes.bitNames.size(); i++) {
            bool bitDone = (i < g_WeeklyStrikes.bitDone.size()) ? g_WeeklyStrikes.bitDone[i] : false;
            const std::string& name = g_WeeklyStrikes.bitNames[i];
            ImGui::PushID((int)i);
            ImVec2 startPos = ImGui::GetCursorScreenPos();
            const char* zone = StrikeZone(name);
            std::string subtitle = "Strike Mission";
            if (zone && *zone) { subtitle += " \xC2\xB7 "; subtitle += zone; }
            RenderClearBanner(name, subtitle,
                g_ClearsFetched, bitDone, StrikeTheme(name),
                /*tooltip*/ nullptr, /*widthOverride*/ halfW,
                /*bannerKey*/ "strike:" + name);
            ImGui::PopID();

            // Place next banner on same row or move to next row
            if (i % 2 == 0 && i + 1 < g_WeeklyStrikes.bitNames.size()) {
                // Reset cursor to right side of just-rendered banner
                ImGui::SetCursorScreenPos(ImVec2(startPos.x + halfW + gap, startPos.y));
            }
        }
        ImGui::Unindent(4.0f);
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
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", weeklyResetStr.c_str());
        RenderSectionHeader("Weekly Raids", ImVec4(0.9f, 0.7f, 0.3f, 1.0f), suffix);
    }

    if (g_WeeklyWings.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        ImGui::Indent(4.0f);
        uint32_t embWingId = CurrentEmboldenedWingId();
        uint32_t cotmWingId = CurrentCallOfTheMistsWingId();
        for (const auto& wing : g_WeeklyWings) {
            std::string cleanName = CleanWingName(wing.name);
            const char* shortW = WingShortName(wing.id);

            // Build chip list, skipping progress-only bits (empty short name)
            std::vector<WingEncChip> chips;
            int doneCount = 0, totalCount = 0;
            for (size_t i = 0; i < wing.bitNames.size(); i++) {
                std::string shortEnc = ShortenEncounterName(wing.bitNames[i]);
                if (shortEnc.empty()) continue; // skip
                bool bitDone = (i < wing.bitDone.size()) ? wing.bitDone[i] : false;
                WingEncChip c;
                c.label = shortEnc;
                c.fullName = wing.bitNames[i];
                c.done = bitDone;
                chips.push_back(c);
                totalCount++;
                if (bitDone) doneCount++;
            }

            ImGui::PushID((int)wing.id);
            RenderWingBanner(shortW ? shortW : "", cleanName,
                doneCount, totalCount, g_ClearsFetched,
                WingTheme(wing.id), chips,
                /*bannerKey*/ "wing:" + std::to_string(wing.id),
                /*isEmboldened*/ wing.id == embWingId,
                /*isCallOfTheMists*/ wing.id == cotmWingId);
            ImGui::PopID();
        }
        ImGui::Unindent(4.0f);
    }
}

void RenderClears() {
    auto now = std::chrono::system_clock::now();

    auto dailyReset  = CalcLastDailyReset(now);
    auto weeklyReset = CalcLastWeeklyReset(now);
    auto dailyBuffered  = dailyReset  + std::chrono::seconds(60);
    auto weeklyBuffered = weeklyReset + std::chrono::seconds(60);

    // ---- Clears reset detection ----
    bool resetTriggered = false;
    if (g_LastDailyReset != std::chrono::system_clock::time_point{} &&
        dailyReset > g_LastDailyReset && now >= dailyBuffered) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_DailyFractals.clear();
        g_DailyBounties.clear();
        g_ClearsFetched = false;
        resetTriggered = true;
    }
    if (g_LastWeeklyReset != std::chrono::system_clock::time_point{} &&
        weeklyReset > g_LastWeeklyReset && now >= weeklyBuffered) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_WeeklyWings.clear();
        g_WeeklyStrikes = ClearEntry{};
        g_ClearsFetched = false;
        resetTriggered = true;
    }
    g_LastDailyReset  = dailyReset;
    g_LastWeeklyReset = weeklyReset;

    if (resetTriggered && !g_ClearsFetching)
        FetchClears();

    // ---- Vault reset detection ----
    if (g_VaultLastDailyReset != std::chrono::system_clock::time_point{} &&
        dailyReset > g_VaultLastDailyReset && now >= dailyBuffered) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_VaultDaily = {};
        g_VaultFetched = false;
    }
    if (g_VaultLastWeeklyReset != std::chrono::system_clock::time_point{} &&
        weeklyReset > g_VaultLastWeeklyReset && now >= weeklyBuffered) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_VaultWeekly = {};
        g_VaultFetched = false;
    }
    g_VaultLastDailyReset  = dailyReset;
    g_VaultLastWeeklyReset = weeklyReset;

    // ---- Clears deferred logic ----
    if (g_ClearsNeedRequery && g_ClearsFetched && !g_ClearsFetching) {
        g_ClearsNeedRequery = false;
        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            for (auto& e : g_DailyFractals) { e.done = false; e.current = 0; e.bitDone.assign(e.bitDone.size(), false); }
            for (auto& e : g_DailyBounties) { e.done = false; e.current = 0; e.bitDone.assign(e.bitDone.size(), false); }
            for (auto& e : g_WeeklyWings)   { e.done = false; e.current = 0; e.bitDone.assign(e.bitDone.size(), false); }
            g_WeeklyStrikes.done = false; g_WeeklyStrikes.current = 0;
            g_WeeklyStrikes.bitDone.assign(g_WeeklyStrikes.bitDone.size(), false);
            g_ClearsStatusMsg = "Refreshing completion for new account...";
        }
        SendClearsAchQuery();
    }
    if (g_ClearsQueryPending.exchange(false) && !g_ClearsFetching)
        SendClearsAchQuery();
    if (g_ClearsRetryPending.exchange(false) && !g_ClearsFetching)
        SendClearsAchQuery();

    auto steadyNow = std::chrono::steady_clock::now();
    if (g_ClearsFetched && !g_ClearsFetching &&
        g_LastClearsCompletionQuery != std::chrono::steady_clock::time_point{} &&
        (steadyNow - g_LastClearsCompletionQuery) >= std::chrono::minutes(10)) {
        g_LastClearsCompletionQuery = steadyNow;
        SendClearsAchQuery();
    }

    // ---- Sub-tabs ----
    std::string dailyResetStr  = FormatTimeUntilReset(dailyReset, std::chrono::hours(24));
    std::string weeklyResetStr = FormatTimeUntilReset(weeklyReset, std::chrono::hours(24 * 7));

    if (ImGui::BeginTabBar("##ClearsSubTabs")) {
        if (ImGui::BeginTabItem("Vault")) {
            RenderVaultTab(dailyResetStr, weeklyResetStr);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Clears")) {
            RenderClearsTabContent(dailyResetStr, weeklyResetStr);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
