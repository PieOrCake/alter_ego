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

    g_VaultFetching = false;
    g_VaultFetched = true;
    SaveVaultCache();
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
        j["daily"]   = serializePeriod(g_VaultDaily);
        j["weekly"]  = serializePeriod(g_VaultWeekly);
        j["special"] = serializePeriod(g_VaultSpecial);
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

// =========================================================================
// Clears - Vault tab rendering
// =========================================================================

static void RenderVaultPeriodSection(const char* title, const char* resetLabel,
                                      ImVec4 color, const VaultPeriod& period)
{
    ImGui::TextColored(color, "%s", title);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%s)", resetLabel);

    if (period.objectives.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
        return;
    }

    // Meta chest progress bar
    if (period.meta_progress_complete > 0) {
        float frac = std::min(1.0f,
            (float)period.meta_progress_current / (float)period.meta_progress_complete);
        ImVec4 barColor = period.meta_reward_claimed
            ? ImVec4(0.35f, 0.82f, 0.35f, 1.0f) : color;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.10f, 0.25f, 1.0f));
        ImGui::ProgressBar(frac, ImVec2(-1.0f, 10.0f), "");
        ImGui::PopStyleColor(2);
    }

    // Objectives
    for (const auto& obj : period.objectives) {
        bool done = obj.isDone();

        ImGui::Text("  ");
        ImGui::SameLine();
        if (done)
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "[x]");
        else
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "[ ]");
        ImGui::SameLine();

        ImVec4 titleColor = obj.claimed ? ImVec4(0.35f, 0.82f, 0.35f, 1.0f)
                          : done        ? ImVec4(0.70f, 0.70f, 0.70f, 1.0f)
                                        : ImVec4(0.90f, 0.90f, 0.90f, 1.0f);
        ImGui::TextColored(titleColor, "%s", obj.title.c_str());

        if (!obj.track.empty()) {
            ImVec4 trackColor = (obj.track == "PvP") ? ImVec4(0.80f, 0.33f, 0.33f, 1.0f)
                              : (obj.track == "WvW") ? ImVec4(0.33f, 0.53f, 0.80f, 1.0f)
                                                     : ImVec4(0.33f, 0.73f, 0.33f, 1.0f);
            ImGui::SameLine();
            ImGui::TextColored(trackColor, "[%s]", obj.track.c_str());
        }

        if (!done && obj.progress_complete > 1) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                "%d/%d", obj.progress_current, obj.progress_complete);
        }

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.91f, 0.78f, 0.29f, 1.0f), "+%d", obj.acclaim);

        if (obj.claimed) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 0.7f), "(claimed)");
        }
    }
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
        RenderVaultPeriodSection("Special Objectives", "seasonal",
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
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", dailyResetStr.c_str());
        RenderSectionHeader("Daily Raid Bounties", ImVec4(0.9f, 0.6f, 0.3f, 1.0f), suffix);
    }

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
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", weeklyResetStr.c_str());
        RenderSectionHeader("Weekly Strikes", ImVec4(0.7f, 0.4f, 0.9f, 1.0f), suffix);
    }

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
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", weeklyResetStr.c_str());
        RenderSectionHeader("Weekly Raids", ImVec4(0.9f, 0.7f, 0.3f, 1.0f), suffix);
    }

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

                // Strip "Weekly " prefix and add W# prefix
                std::string wingName = wing.name;
                if (wingName.find("Weekly ") == 0)
                    wingName = wingName.substr(7);
                {
                    static const std::unordered_map<uint32_t, int> wingNum = {
                        {9128, 1}, {9147, 2}, {9182, 3}, {9144, 4},
                        {9111, 5}, {9120, 6}, {9156, 7}, {9181, 8},
                    };
                    auto it = wingNum.find(wing.id);
                    if (it != wingNum.end())
                        wingName = "W" + std::to_string(it->second) + ": " + wingName;
                }

                ImGui::Text("%s", wingName.c_str());

                ImGui::TableNextColumn();
                // Render encounters, skipping progress-only bits (empty short name)
                bool firstEnc = true;
                for (size_t i = 0; i < wing.bitNames.size(); i++) {
                    std::string shortName = ShortenEncounterName(wing.bitNames[i]);
                    if (shortName.empty()) continue; // skip progress-only bits
                    bool bitDone = (i < wing.bitDone.size()) ? wing.bitDone[i] : false;
                    if (!firstEnc) {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "|");
                        ImGui::SameLine();
                    }
                    RenderClearStatus(g_ClearsFetched, bitDone, shortName.c_str());
                    firstEnc = false;
                }
            }
            ImGui::EndTable();
        }
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
