#include "GW2API.h"
#include "HoardAndSeekAPI.h"

#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <unordered_set>
#include <set>

using json = nlohmann::json;

namespace AlterEgo {

    // Strip HTML-like tags from GW2 API descriptions
    static std::string StripHtmlTags(const std::string& input) {
        std::string result;
        result.reserve(input.size());
        bool in_tag = false;
        for (size_t i = 0; i < input.size(); i++) {
            if (input[i] == '<') {
                if (i + 3 < input.size() && input[i+1] == 'b' && input[i+2] == 'r' && input[i+3] == '>') {
                    result += '\n';
                    i += 3;
                    continue;
                }
                in_tag = true;
            } else if (input[i] == '>') {
                in_tag = false;
            } else if (!in_tag) {
                result += input[i];
            }
        }
        return result;
    }

    // URL-encode helper
    static std::string UrlEncode(const std::string& name) {
        std::string encoded;
        for (char c : name) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded += c;
            } else {
                char hex[4];
                snprintf(hex, sizeof(hex), "%%%02X", (unsigned char)c);
                encoded += hex;
            }
        }
        return encoded;
    }

    // Static member initialization
    AddonAPI_t* GW2API::s_api = nullptr;
    HoardStatus GW2API::s_hoard_status = HoardStatus::Unknown;
    FetchStatus GW2API::s_fetch_status = FetchStatus::Idle;
    std::string GW2API::s_fetch_message;
    std::vector<Character> GW2API::s_characters;
    bool GW2API::s_has_character_data = false;
    time_t GW2API::s_last_updated = 0;
    std::vector<std::string> GW2API::s_pending_char_names;
    int GW2API::s_chars_fetched = 0;
    int GW2API::s_chars_total = 0;
    std::string GW2API::s_priority_char_name;
    int GW2API::s_pending_char_idx = 0;
    bool GW2API::s_list_only_mode = false;
    Character GW2API::s_current_char;
    int GW2API::s_fetch_phase = 0;
    std::vector<int> GW2API::s_equip_tab_ids;
    int GW2API::s_equip_tab_idx = 0;
    std::unordered_map<uint32_t, bool> GW2API::s_skin_unlocks;
    std::unordered_map<uint32_t, ItemLocationResult> GW2API::s_item_locations;
    std::unordered_map<uint32_t, ItemInfo> GW2API::s_item_cache;
    std::unordered_map<uint32_t, SkinInfo> GW2API::s_skin_cache;
    std::unordered_map<uint32_t, SpecializationInfo> GW2API::s_spec_cache;
    std::unordered_map<uint32_t, TraitInfo> GW2API::s_trait_cache;
    std::unordered_map<uint32_t, SkillInfo> GW2API::s_skill_cache;
    std::unordered_map<int, DyeColor> GW2API::s_dye_cache;
    std::unordered_map<uint32_t, ItemStatInfo> GW2API::s_itemstat_cache;
    std::unordered_map<std::string, ProfessionInfo> GW2API::s_profession_cache;
    std::unordered_map<std::string, std::map<std::string, ProfessionWeaponData>> GW2API::s_profession_weapons;
    std::vector<SavedBuild> GW2API::s_saved_builds;
    std::unordered_map<std::string, uint32_t> GW2API::s_item_name_id_cache;
    bool GW2API::s_item_name_cache_dirty = false;
    std::unordered_map<std::string, std::unordered_map<uint16_t, uint32_t>> GW2API::s_palette_to_skill;
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint16_t>> GW2API::s_skill_to_palette;
    std::unordered_set<std::string> GW2API::s_palette_fetching;
    std::mutex GW2API::s_mutex;

    // --- Nexus integration ---

    void GW2API::Initialize(AddonAPI_t* api) {
        s_api = api;
    }

    void GW2API::Shutdown() {
        s_api = nullptr;
    }

    // Helper: get the DLL directory
    static std::string GetDllDir() {
        char dllPath[MAX_PATH];
        HMODULE hModule = NULL;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)GetDllDir, &hModule)) {
            if (GetModuleFileNameA(hModule, dllPath, MAX_PATH)) {
                std::string path(dllPath);
                size_t lastSlash = path.find_last_of("\\/");
                if (lastSlash != std::string::npos) {
                    return path.substr(0, lastSlash);
                }
            }
        }
        return "";
    }

    std::string GW2API::GetDataDirectory() {
        std::string dir = GetDllDir();
        if (!dir.empty()) {
            std::replace(dir.begin(), dir.end(), '\\', '/');
        }
        return dir + "/AlterEgo";
    }

    bool GW2API::EnsureDataDirectory() {
        std::string dir = GetDataDirectory();
        try {
            std::filesystem::create_directories(dir);
            return true;
        } catch (...) {
            return false;
        }
    }

    // =========================================================================
    // H&S Integration
    // =========================================================================

    HoardStatus GW2API::GetHoardStatus() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_hoard_status;
    }

    void GW2API::PingHoard() {
        if (!s_api) return;
        s_api->Events_Raise(EV_HOARD_PING, nullptr);
    }

    void GW2API::OnHoardPong(void* eventArgs) {
        if (!eventArgs) return;
        auto* pong = (HoardPongPayload*)eventArgs;
        if (pong->api_version != HOARD_API_VERSION) return;

        std::lock_guard<std::mutex> lock(s_mutex);
        s_hoard_status = HoardStatus::Available;
        if (s_api) {
            s_api->Log(LOGL_INFO, "AlterEgo",
                pong->has_data ? "Hoard & Seek is available (has data)" :
                                 "Hoard & Seek is available (no data yet)");
        }
    }

    void GW2API::OnHoardDataUpdated(void* eventArgs) {
        if (!eventArgs) return;
        auto* payload = (HoardDataReadyPayload*)eventArgs;
        if (payload->api_version != HOARD_API_VERSION) return;

        // H&S refreshed its data — auto-refresh our character data
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_hoard_status == HoardStatus::Available || s_hoard_status == HoardStatus::Ready) {
            s_hoard_status = HoardStatus::Ready;
        }
        if (s_api) {
            s_api->Log(LOGL_INFO, "AlterEgo", "H&S data updated, refreshing characters...");
        }
        // Don't hold lock while raising events — schedule refresh on next frame
        // (handled in dllmain render loop)
    }

    // --- H&S cached queries (synchronous) ---

    void GW2API::QuerySkinUnlocks(const std::vector<uint32_t>& skin_ids) {
        if (!s_api || skin_ids.empty()) return;

        // Filter out already-queried skins
        std::vector<uint32_t> to_query;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (uint32_t id : skin_ids) {
                if (id != 0 && s_skin_unlocks.find(id) == s_skin_unlocks.end())
                    to_query.push_back(id);
            }
        }
        if (to_query.empty()) return;

        std::sort(to_query.begin(), to_query.end());
        to_query.erase(std::unique(to_query.begin(), to_query.end()), to_query.end());

        // Batch in groups of 200 (H&S limit)
        // NOTE: EV_HOARD_QUERY_SKINS is synchronous — response handler runs
        // inline before Events_Raise returns. Do NOT hold s_mutex here.
        const size_t BATCH = 200;
        for (size_t i = 0; i < to_query.size(); i += BATCH) {
            HoardQuerySkinsRequest req{};
            req.api_version = HOARD_API_VERSION;
            strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
            strncpy(req.response_event, EV_AE_SKIN_UNLOCK_RESP, sizeof(req.response_event) - 1);
            req.id_count = (uint32_t)std::min(BATCH, to_query.size() - i);
            for (uint32_t j = 0; j < req.id_count; j++)
                req.ids[j] = to_query[i + j];
            s_api->Events_Raise(EV_HOARD_QUERY_SKINS, &req);
        }
    }

    void GW2API::OnSkinUnlocksResponse(void* eventArgs) {
        if (!eventArgs) return;
        auto* resp = (HoardQuerySkinsResponse*)eventArgs;
        if (resp->api_version != HOARD_API_VERSION) return;
        if (resp->status != HOARD_STATUS_OK) {
            delete resp;
            return;
        }

        std::lock_guard<std::mutex> lock(s_mutex);
        for (uint32_t i = 0; i < resp->entry_count; i++) {
            s_skin_unlocks[resp->entries[i].id] = (resp->entries[i].unlocked != 0);
        }
        delete resp;
    }

    void GW2API::QueryItemLocation(uint32_t item_id) {
        if (!s_api || item_id == 0) return;

        // NOTE: Synchronous dispatch — do NOT hold s_mutex here.
        HoardQueryItemRequest req{};
        req.api_version = HOARD_API_VERSION;
        strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
        req.item_id = item_id;
        strncpy(req.response_event, EV_AE_ITEM_LOC_RESP, sizeof(req.response_event) - 1);
        s_api->Events_Raise(EV_HOARD_QUERY_ITEM, &req);
    }

    void GW2API::OnItemLocationResponse(void* eventArgs) {
        if (!eventArgs) return;
        auto* resp = (HoardQueryItemResponse*)eventArgs;
        if (resp->api_version != HOARD_API_VERSION) return;
        if (resp->status != HOARD_STATUS_OK) {
            delete resp;
            return;
        }

        ItemLocationResult result;
        result.item_id = resp->item_id;
        result.name = resp->name;
        result.total_count = resp->total_count;
        for (uint32_t i = 0; i < resp->location_count && i < 32; i++) {
            ItemLocationEntry entry;
            entry.location = resp->locations[i].location;
            entry.sublocation = resp->locations[i].sublocation;
            entry.count = resp->locations[i].count;
            result.locations.push_back(entry);
        }

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_item_locations[resp->item_id] = std::move(result);
        }
        delete resp;
    }

    bool GW2API::IsSkinUnlocked(uint32_t skin_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_skin_unlocks.find(skin_id);
        return (it != s_skin_unlocks.end()) ? it->second : false;
    }

    bool GW2API::HasSkinUnlockData(uint32_t skin_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_skin_unlocks.find(skin_id) != s_skin_unlocks.end();
    }

    const ItemLocationResult* GW2API::GetItemLocation(uint32_t item_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_item_locations.find(item_id);
        return (it != s_item_locations.end()) ? &it->second : nullptr;
    }

    // --- Character data fetch via H&S API proxy ---

    void GW2API::RequestCharacterRefresh(const std::string& priorityChar) {
        if (!s_api) return;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            // Allow re-entry during PermPending (retry after permission popup)
            if (s_fetch_status == FetchStatus::InProgress &&
                s_hoard_status != HoardStatus::PermPending) return;
            if (s_hoard_status != HoardStatus::Available &&
                s_hoard_status != HoardStatus::Ready &&
                s_hoard_status != HoardStatus::PermPending) {
                s_fetch_message = "Hoard & Seek not available";
                s_fetch_status = FetchStatus::Error;
                return;
            }
            s_fetch_status = FetchStatus::InProgress;
            s_fetch_message = "Requesting character list from H&S...";
            s_pending_char_names.clear();
            s_pending_char_idx = 0;
            s_chars_fetched = 0;
            s_chars_total = 0;
            s_priority_char_name = priorityChar;
            s_list_only_mode = false;
        }

        // Request character list via H&S API proxy
        HoardQueryApiRequest req{};
        req.api_version = HOARD_API_VERSION;
        strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
        strncpy(req.endpoint, "/v2/characters", sizeof(req.endpoint) - 1);
        strncpy(req.response_event, EV_AE_CHAR_LIST_RESP, sizeof(req.response_event) - 1);
        s_api->Events_Raise(EV_HOARD_QUERY_API, &req);
    }

    void GW2API::RequestCharacterList() {
        if (!s_api) return;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_fetch_status == FetchStatus::InProgress &&
                s_hoard_status != HoardStatus::PermPending) return;
            if (s_hoard_status != HoardStatus::Available &&
                s_hoard_status != HoardStatus::Ready &&
                s_hoard_status != HoardStatus::PermPending) {
                s_fetch_message = "Hoard & Seek not available";
                s_fetch_status = FetchStatus::Error;
                return;
            }
            s_fetch_status = FetchStatus::InProgress;
            s_fetch_message = "Fetching character list...";
            s_list_only_mode = true;
        }

        HoardQueryApiRequest req{};
        req.api_version = HOARD_API_VERSION;
        strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
        strncpy(req.endpoint, "/v2/characters", sizeof(req.endpoint) - 1);
        strncpy(req.response_event, EV_AE_CHAR_LIST_RESP, sizeof(req.response_event) - 1);
        s_api->Events_Raise(EV_HOARD_QUERY_API, &req);
    }

    void GW2API::RequestCharacterRefreshSelected(const std::vector<std::string>& names) {
        if (!s_api || names.empty()) return;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_fetch_status = FetchStatus::InProgress;
            s_pending_char_names = names;
            s_pending_char_idx = 0;
            s_chars_total = (int)names.size();
            s_chars_fetched = 0;
            s_priority_char_name.clear();
            s_list_only_mode = false;

            // Start fetching the first character
            s_current_char = Character{};
            s_current_char.name = s_pending_char_names[0];
            s_fetch_phase = 0;
            s_equip_tab_ids.clear();
            s_equip_tab_idx = 0;
            s_fetch_message = s_current_char.name + ": Fetching (1/" +
                std::to_string(s_chars_total) + ")...";
        }

        if (s_api) {
            s_api->Log(LOGL_INFO, "AlterEgo",
                ("Fetching " + std::to_string(names.size()) + " selected characters").c_str());
        }

        FetchCharacterPhase();
    }

    void GW2API::OnCharListResponse(void* eventArgs) {
        if (!eventArgs) {
            if (s_api) s_api->Log(LOGL_WARNING, "AlterEgo", "OnCharListResponse: eventArgs is null");
            return;
        }
        auto* resp = (HoardQueryApiResponse*)eventArgs;
        if (resp->api_version != HOARD_API_VERSION) {
            delete resp;
            return;
        }

        if (s_api) {
            std::string msg = "OnCharListResponse: status=" + std::to_string(resp->status) +
                " json_length=" + std::to_string(resp->json_length);
            s_api->Log(LOGL_INFO, "AlterEgo", msg.c_str());
        }

        // Check permission status
        if (resp->status == HOARD_STATUS_PENDING) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_hoard_status = HoardStatus::PermPending;
            s_fetch_message = "Waiting for H&S permission approval...";
            s_fetch_status = FetchStatus::InProgress;
            delete resp;
            return;
        }
        if (resp->status == HOARD_STATUS_DENIED) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_hoard_status = HoardStatus::PermDenied;
            s_fetch_message = "H&S permission denied. Enable in H&S settings.";
            s_fetch_status = FetchStatus::Error;
            delete resp;
            return;
        }
        if (resp->status != HOARD_STATUS_OK || resp->truncated) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_fetch_message = resp->truncated ? "Character list truncated" : "H&S returned error";
            s_fetch_status = FetchStatus::Error;
            delete resp;
            return;
        }

        // Parse character names from JSON array (use json_length!)
        try {
            json j = json::parse(resp->json, resp->json + resp->json_length);
            if (!j.is_array() || j.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_fetch_message = "No characters found";
                s_fetch_status = FetchStatus::Error;
                delete resp;
                return;
            }

            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_hoard_status = HoardStatus::Ready;

                // Collect ALL character names
                s_pending_char_names.clear();
                for (const auto& name : j) {
                    s_pending_char_names.push_back(name.get<std::string>());
                }

                // List-only mode: just store names and return
                if (s_list_only_mode) {
                    s_list_only_mode = false;
                    s_fetch_message = "Character list received";
                    s_fetch_status = FetchStatus::Success;
                    delete resp;
                    return;
                }

                // If a priority character is specified, move it to the front
                if (!s_priority_char_name.empty()) {
                    for (size_t i = 1; i < s_pending_char_names.size(); i++) {
                        if (s_pending_char_names[i] == s_priority_char_name) {
                            std::swap(s_pending_char_names[0], s_pending_char_names[i]);
                            break;
                        }
                    }
                }

                s_chars_total = (int)s_pending_char_names.size();
                s_chars_fetched = 0;
                s_pending_char_idx = 0;
                // Don't clear s_characters — keep cached data visible while refreshing

                // Initialize sub-endpoint fetch for the first character
                s_current_char = Character{};
                s_current_char.name = s_pending_char_names[0];
                s_fetch_phase = 0;
                s_equip_tab_ids.clear();
                s_equip_tab_idx = 0;
                s_fetch_message = s_current_char.name + ": Fetching (1/" +
                    std::to_string(s_chars_total) + ")...";
            }

            if (s_api) {
                s_api->Log(LOGL_INFO, "AlterEgo",
                    ("Fetching " + std::to_string(j.size()) + " characters, starting with: " +
                     s_pending_char_names[0]).c_str());
            }

            // Start sub-endpoint fetch (phase 0 = /core)
            FetchCharacterPhase();

        } catch (...) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_fetch_message = "Failed to parse character list";
            s_fetch_status = FetchStatus::Error;
        }

        delete resp;
    }

    void GW2API::FetchCharacterPhase() {
        if (!s_api) return;

        std::string char_name;
        int phase;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            char_name = s_current_char.name;
            phase = s_fetch_phase;
        }

        // Phase 0: /core
        // Phase 1: /equipmenttabs (just tab ID list)
        // Phase 2: /equipmenttabs/:N (one per tab, repeats)
        // Phase 3: /equipment (supplemental: PowerCore/Relic + tool slots not in equipmenttabs)
        // Phase 4: /buildtabs?tabs=all
        std::string endpoint;
        switch (phase) {
            case 0:
                endpoint = "/v2/characters/" + UrlEncode(char_name) + "/core";
                break;
            case 1:
                endpoint = "/v2/characters/" + UrlEncode(char_name) + "/equipmenttabs";
                break;
            case 2: {
                std::lock_guard<std::mutex> lock(s_mutex);
                if (s_equip_tab_idx >= (int)s_equip_tab_ids.size()) {
                    // All equipment tabs fetched, advance to /equipment for supplemental slots
                    s_fetch_phase = 3;
                    phase = 3;
                } else {
                    int tab_id = s_equip_tab_ids[s_equip_tab_idx];
                    endpoint = "/v2/characters/" + UrlEncode(char_name) +
                        "/equipmenttabs/" + std::to_string(tab_id) +
                        "?v=2024-04-01T00:00:00.000Z";
                }
                break;
            }
            case 3:
                endpoint = "/v2/characters/" + UrlEncode(char_name) + "/equipment?v=2024-04-01T00:00:00.000Z";
                break;
            case 4:
                endpoint = "/v2/characters/" + UrlEncode(char_name) + "/buildtabs?tabs=all&v=2021-07-15T13:00:00.000Z";
                break;
            case 5:
                endpoint = "/v2/characters/" + UrlEncode(char_name) + "/crafting";
                break;
            default:
                return;
        }

        // Phase 2 may have advanced to 3 above
        if (phase == 3) {
            endpoint = "/v2/characters/" + UrlEncode(char_name) + "/equipment?v=2024-04-01T00:00:00.000Z";
        }

        if (s_api) {
            s_api->Log(LOGL_INFO, "AlterEgo", ("Fetching: " + endpoint).c_str());
        }

        HoardQueryApiRequest req{};
        req.api_version = HOARD_API_VERSION;
        strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
        strncpy(req.endpoint, endpoint.c_str(), sizeof(req.endpoint) - 1);
        req.endpoint[sizeof(req.endpoint) - 1] = '\0';
        strncpy(req.response_event, EV_AE_CHAR_DATA_RESP, sizeof(req.response_event) - 1);
        s_api->Events_Raise(EV_HOARD_QUERY_API, &req);
    }

    // Safe helpers: get value from JSON, return default if null or missing
    static int SafeInt(const json& j, const char* key, int def = 0) {
        if (!j.contains(key) || j[key].is_null()) return def;
        return j[key].get<int>();
    }
    static uint32_t SafeUint(const json& j, const char* key, uint32_t def = 0) {
        if (!j.contains(key) || j[key].is_null()) return def;
        return j[key].get<uint32_t>();
    }
    static bool SafeBool(const json& j, const char* key, bool def = false) {
        if (!j.contains(key) || j[key].is_null()) return def;
        return j[key].get<bool>();
    }
    static std::string SafeStr(const json& j, const char* key, const std::string& def = "") {
        if (!j.contains(key) || j[key].is_null()) return def;
        return j[key].get<std::string>();
    }

    void GW2API::OnCharDataResponse(void* eventArgs) {
        if (!eventArgs) {
            if (s_api) s_api->Log(LOGL_WARNING, "AlterEgo", "OnCharDataResponse: eventArgs is null");
            return;
        }
        auto* resp = (HoardQueryApiResponse*)eventArgs;
        if (resp->api_version != HOARD_API_VERSION) {
            delete resp;
            return;
        }

        int phase;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            phase = s_fetch_phase;
        }

        if (s_api) {
            std::string msg = "OnCharDataResponse: phase=" + std::to_string(phase) +
                " status=" + std::to_string(resp->status) +
                " truncated=" + std::to_string(resp->truncated) +
                " json_length=" + std::to_string(resp->json_length);
            s_api->Log(LOGL_INFO, "AlterEgo", msg.c_str());
        }

        if (resp->status != HOARD_STATUS_OK) {
            if (s_api) s_api->Log(LOGL_WARNING, "AlterEgo", "Sub-endpoint fetch failed");
            std::lock_guard<std::mutex> lock(s_mutex);
            s_fetch_message = "Failed to fetch character data";
            s_fetch_status = FetchStatus::Error;
            delete resp;
            return;
        }

        if (resp->truncated) {
            if (s_api) {
                std::string msg = "Sub-endpoint truncated (" + std::to_string(resp->json_length) + " bytes)";
                s_api->Log(LOGL_WARNING, "AlterEgo", msg.c_str());
            }
        }

        // For truncated responses, attempt to recover by closing open brackets
        std::string json_str;
        uint32_t parse_len = resp->truncated
            ? std::min(resp->json_length, (uint32_t)sizeof(((HoardQueryApiResponse*)0)->json))
            : resp->json_length;
        if (resp->truncated) {
            json_str.assign(resp->json, parse_len);
            // Find the last complete top-level object in the array.
            // The /equipment response is {"equipment":[{...},{...},...]}
            // We search backward for '}' at brace depth 0 (end of a complete object).
            size_t last_complete = std::string::npos;
            int depth = 0;
            bool in_str = false;
            for (size_t i = 0; i < json_str.size(); i++) {
                char c = json_str[i];
                if (c == '\\' && in_str) { i++; continue; } // skip escaped char
                if (c == '"') { in_str = !in_str; continue; }
                if (in_str) continue;
                if (c == '{') depth++;
                else if (c == '}') {
                    depth--;
                    if (depth == 1) // closing a top-level equipment object (depth 1 = inside the array)
                        last_complete = i;
                }
            }
            if (last_complete != std::string::npos) {
                json_str.resize(last_complete + 1);
                json_str += "]}"; // close the equipment array and outer object
            } else {
                // Couldn't find a complete object — wrap as empty
                json_str = "{\"equipment\":[]}";
            }
            if (s_api) {
                s_api->Log(LOGL_INFO, "AlterEgo",
                    ("Truncation recovery: trimmed to " + std::to_string(json_str.size()) + " bytes").c_str());
            }
        }

        try {
            json cj = resp->truncated
                ? json::parse(json_str)
                : json::parse(resp->json, resp->json + resp->json_length);

            // Check for API error response
            if (cj.is_object() && cj.contains("text")) {
                if (s_api) {
                    s_api->Log(LOGL_WARNING, "AlterEgo",
                        ("API error: " + cj["text"].get<std::string>()).c_str());
                }
                std::lock_guard<std::mutex> lock(s_mutex);
                s_fetch_message = "API error: " + cj["text"].get<std::string>();
                s_fetch_status = FetchStatus::Error;
                delete resp;
                return;
            }

            switch (phase) {
                case 0: { // /core — basic character info
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_current_char.name = cj.value("name", s_current_char.name);
                    s_current_char.race = cj.value("race", "");
                    s_current_char.gender = cj.value("gender", "");
                    s_current_char.profession = cj.value("profession", "");
                    s_current_char.level = cj.value("level", 0);
                    s_current_char.age = cj.value("age", 0);
                    s_current_char.created = cj.value("created", "");
                    s_current_char.last_modified = cj.value("last_modified", "");
                    s_current_char.deaths = cj.value("deaths", 0);
                    // Crafting is fetched separately in phase 5 (/crafting)
                    break;
                }
                case 1: { // /equipmenttabs — just tab ID list [1,2,3,4,5]
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_equip_tab_ids.clear();
                    s_equip_tab_idx = 0;
                    if (cj.is_array()) {
                        for (const auto& id : cj) {
                            if (id.is_number()) s_equip_tab_ids.push_back(id.get<int>());
                        }
                    }
                    if (s_api) {
                        s_api->Log(LOGL_INFO, "AlterEgo",
                            ("Found " + std::to_string(s_equip_tab_ids.size()) + " equipment tabs").c_str());
                    }
                    break;
                }
                case 2: { // /equipmenttabs/:N — single tab data
                    std::lock_guard<std::mutex> lock(s_mutex);
                    int tab_num = SafeInt(cj, "tab", 0);
                    if (cj.contains("is_active") && !cj["is_active"].is_null() && cj["is_active"].get<bool>()) {
                        s_current_char.active_equipment_tab = tab_num;
                    }
                    // Store tab name
                    std::string tab_name = SafeStr(cj, "name");
                    if (!tab_name.empty()) {
                        s_current_char.equipment_tab_names[tab_num] = tab_name;
                    }
                    if (cj.contains("equipment") && cj["equipment"].is_array()) {
                        for (const auto& eq_json : cj["equipment"]) {
                            EquipmentItem eq = ParseEquipmentItem(eq_json);
                            eq.tab = tab_num;
                            s_current_char.equipment.push_back(eq);
                        }
                    }
                    s_equip_tab_idx++;
                    if (s_api) {
                        // Log all slot names for debugging
                        std::string slots_str;
                        for (const auto& e : s_current_char.equipment) {
                            if (e.tab == tab_num && !e.slot.empty()) {
                                if (!slots_str.empty()) slots_str += ", ";
                                slots_str += e.slot;
                            }
                        }
                        s_api->Log(LOGL_INFO, "AlterEgo",
                            ("Tab " + std::to_string(tab_num) + " slots: " + slots_str).c_str());
                        s_api->Log(LOGL_INFO, "AlterEgo",
                            ("Tab " + std::to_string(tab_num) + ": " +
                             std::to_string(s_current_char.equipment.size()) + " items total").c_str());
                    }
                    break;
                }
                case 3: { // /equipment — supplemental: PowerCore (Relic) + tool slots
                    std::lock_guard<std::mutex> lock(s_mutex);
                    // Slots only available in /equipment, not in /equipmenttabs
                    static const std::set<std::string> SUPPLEMENTAL_SLOTS = {
                        "Relic", "PowerCore", "Sickle", "Axe", "Pick",
                        "FishingRod", "FishingBait", "FishingLure",
                        "SensoryArray", "ServiceChip"
                    };
                    if (cj.contains("equipment") && cj["equipment"].is_array()) {
                        int relic_count = 0;
                        for (const auto& eq_json : cj["equipment"]) {
                            std::string slot = SafeStr(eq_json, "slot");
                            if (slot.empty()) continue;
                            if (SUPPLEMENTAL_SLOTS.count(slot) == 0) continue;
                            EquipmentItem eq = ParseEquipmentItem(eq_json);
                            // Use tabs array to add item to each tab it belongs to
                            if (eq_json.contains("tabs") && eq_json["tabs"].is_array()) {
                                for (const auto& t : eq_json["tabs"]) {
                                    if (t.is_null()) continue;
                                    EquipmentItem eq_copy = eq;
                                    eq_copy.tab = t.get<int>();
                                    s_current_char.equipment.push_back(eq_copy);
                                    if (slot == "PowerCore") relic_count++;
                                }
                            } else {
                                // No tabs array — shared across all tabs (tab=0)
                                eq.tab = 0;
                                s_current_char.equipment.push_back(eq);
                                if (slot == "PowerCore") relic_count++;
                            }
                        }
                        if (s_api) {
                            s_api->Log(LOGL_INFO, "AlterEgo",
                                ("Supplemental: found " + std::to_string(relic_count) + " relic entries").c_str());
                        }
                    }
                    break;
                }
                case 4: { // /buildtabs?tabs=all — all build tabs
                    std::lock_guard<std::mutex> lock(s_mutex);
                    if (cj.is_array()) {
                        for (const auto& tab : cj) {
                            if (tab.is_null()) continue;
                            s_current_char.build_tabs.push_back(ParseBuildTemplate(tab));
                            if (tab.contains("is_active") && !tab["is_active"].is_null() &&
                                tab["is_active"].get<bool>()) {
                                s_current_char.active_build_tab = SafeInt(tab, "tab", 0);
                            }
                        }
                    }
                    break;
                }
                case 5: { // /crafting
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_current_char.crafting.clear();
                    s_current_char.crafting_levels.clear();
                    s_current_char.crafting_active.clear();
                    // Response is {"crafting": [...]} or just an array
                    json craftArr = cj;
                    if (cj.contains("crafting") && cj["crafting"].is_array())
                        craftArr = cj["crafting"];
                    if (craftArr.is_array()) {
                        // Collect active first, then inactive, all with rating >= 1
                        std::vector<std::tuple<std::string, int, bool>> allCrafts;
                        for (const auto& craft : craftArr) {
                            int rating = craft.value("rating", 0);
                            if (rating >= 1) {
                                allCrafts.emplace_back(
                                    craft.value("discipline", ""),
                                    rating,
                                    craft.value("active", false));
                            }
                        }
                        // Sort: active first
                        std::sort(allCrafts.begin(), allCrafts.end(),
                            [](const auto& a, const auto& b) {
                                return std::get<2>(a) > std::get<2>(b);
                            });
                        for (const auto& [disc, lvl, active] : allCrafts) {
                            s_current_char.crafting.push_back(disc);
                            s_current_char.crafting_levels.push_back(lvl);
                            s_current_char.crafting_active.push_back(active);
                        }
                    }
                    break;
                }
            }
        } catch (const std::exception& e) {
            if (s_api) {
                std::string msg = "Parse error phase " + std::to_string(phase) + ": " + e.what();
                s_api->Log(LOGL_WARNING, "AlterEgo", msg.c_str());
            }
        }

        delete resp;

        // Determine next action
        bool done = false;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            switch (phase) {
                case 0: // core done → fetch equip tab IDs
                    s_fetch_phase = 1;
                    s_fetch_message = s_current_char.name + ": Fetching equipment tabs...";
                    break;
                case 1: // tab IDs done → start fetching individual tabs
                    s_fetch_phase = 2;
                    s_equip_tab_idx = 0;
                    if (s_equip_tab_ids.empty()) {
                        // No equipment tabs, skip to supplemental /equipment
                        s_fetch_phase = 3;
                        s_fetch_message = s_current_char.name + ": Fetching relic & tools...";
                    } else {
                        s_fetch_message = s_current_char.name + ": Fetching equipment tab 1/" +
                            std::to_string(s_equip_tab_ids.size()) + "...";
                    }
                    break;
                case 2: // individual equip tab done
                    if (s_equip_tab_idx >= (int)s_equip_tab_ids.size()) {
                        // All equipment tabs done → supplemental /equipment
                        s_fetch_phase = 3;
                        s_fetch_message = s_current_char.name + ": Fetching relic & tools...";
                    } else {
                        s_fetch_message = s_current_char.name + ": Fetching equipment tab " +
                            std::to_string(s_equip_tab_idx + 1) + "/" +
                            std::to_string(s_equip_tab_ids.size()) + "...";
                    }
                    break;
                case 3: // supplemental /equipment done → buildtabs
                    s_fetch_phase = 4;
                    s_fetch_message = s_current_char.name + ": Fetching builds...";
                    break;
                case 4: // buildtabs done → crafting
                    s_fetch_phase = 5;
                    s_fetch_message = s_current_char.name + ": Fetching crafting...";
                    break;
                case 5: { // crafting done → add/update this character, then next
                    // Add or update this character in the list
                    bool found = false;
                    for (auto& existing : s_characters) {
                        if (existing.name == s_current_char.name) {
                            existing = s_current_char;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        s_characters.push_back(s_current_char);
                    }
                    s_has_character_data = true;
                    s_last_updated = std::time(nullptr);
                    s_chars_fetched++;
                    s_pending_char_idx++;

                    if (s_pending_char_idx < (int)s_pending_char_names.size()) {
                        // More characters to fetch — start the next one
                        s_current_char = Character{};
                        s_current_char.name = s_pending_char_names[s_pending_char_idx];
                        s_fetch_phase = 0;
                        s_equip_tab_ids.clear();
                        s_equip_tab_idx = 0;
                        s_fetch_message = s_current_char.name + ": Fetching (" +
                            std::to_string(s_chars_fetched + 1) + "/" +
                            std::to_string(s_chars_total) + ")...";
                    } else {
                        // All selected characters done
                        done = true;
                        s_fetch_message = "Done! " + std::to_string(s_chars_total) + " characters loaded.";
                        s_fetch_status = FetchStatus::Success;
                    }
                    break;
                }
            }
        }

        if (done) {
            if (s_api) s_api->Log(LOGL_INFO, "AlterEgo", s_fetch_message.c_str());
            SaveCharacterData();
            SaveItemNameCache();
        } else {
            // Save after each character completes (for incremental persistence)
            if (phase == 5) SaveCharacterData();
            FetchCharacterPhase();
        }
    }

    // --- Parse helpers ---

    EquipmentItem GW2API::ParseEquipmentItem(const json& item_json) {
        EquipmentItem eq;
        eq.id = SafeUint(item_json, "id");
        eq.slot = SafeStr(item_json, "slot");
        eq.skin = SafeUint(item_json, "skin");
        eq.binding = SafeStr(item_json, "binding");
        eq.bound_to = SafeStr(item_json, "bound_to");

        if (item_json.contains("upgrades") && item_json["upgrades"].is_array()) {
            for (const auto& u : item_json["upgrades"])
                eq.upgrades.push_back(u.get<uint32_t>());
        }
        if (item_json.contains("infusions") && item_json["infusions"].is_array()) {
            for (const auto& inf : item_json["infusions"])
                eq.infusions.push_back(inf.get<uint32_t>());
        }
        if (item_json.contains("dyes") && item_json["dyes"].is_array()) {
            for (const auto& d : item_json["dyes"])
                eq.dyes.push_back(d.is_null() ? 0 : d.get<int>());
        }

        if (item_json.contains("tabs") && item_json["tabs"].is_array() && !item_json["tabs"].empty()) {
            eq.tab = item_json["tabs"][0].get<int>();
        } else {
            eq.tab = item_json.value("tab", 0);
        }

        // Parse stats (selected stat set for legendary/ascended items)
        if (item_json.contains("stats") && item_json["stats"].is_object()) {
            const auto& stats = item_json["stats"];
            eq.stat_id = SafeUint(stats, "id");
            if (stats.contains("attributes") && stats["attributes"].is_object()) {
                for (auto it = stats["attributes"].begin(); it != stats["attributes"].end(); ++it) {
                    eq.attributes[it.key()] = it.value().get<int>();
                }
            }
        }

        return eq;
    }

    BuildTemplate GW2API::ParseBuildTemplate(const json& tab_json) {
        BuildTemplate bt;
        bt.tab = SafeInt(tab_json, "tab");
        bt.is_active = SafeBool(tab_json, "is_active");

        if (tab_json.contains("build") && !tab_json["build"].is_null()) {
            const auto& build = tab_json["build"];
            bt.name = SafeStr(build, "name");
            bt.profession = SafeStr(build, "profession");

            if (build.contains("specializations") && build["specializations"].is_array()) {
                int idx = 0;
                for (const auto& spec : build["specializations"]) {
                    if (idx >= 3) break;
                    if (spec.is_null()) { idx++; continue; }
                    bt.specializations[idx].spec_id = SafeUint(spec, "id");
                    if (spec.contains("traits") && spec["traits"].is_array()) {
                        int ti = 0;
                        for (const auto& t : spec["traits"]) {
                            if (ti >= 3) break;
                            bt.specializations[idx].traits[ti] = t.is_null() ? 0 : t.get<int>();
                            ti++;
                        }
                    }
                    idx++;
                }
            }

            auto parseSkillBar = [](const json& skills_json) -> SkillBar {
                SkillBar sb;
                if (skills_json.is_null()) return sb;
                sb.heal = SafeUint(skills_json, "heal");
                if (skills_json.contains("utilities") && skills_json["utilities"].is_array()) {
                    int i = 0;
                    for (const auto& u : skills_json["utilities"]) {
                        if (i >= 3) break;
                        sb.utilities[i] = u.is_null() ? 0 : u.get<uint32_t>();
                        i++;
                    }
                }
                sb.elite = SafeUint(skills_json, "elite");
                return sb;
            };

            if (build.contains("skills"))
                bt.terrestrial_skills = parseSkillBar(build["skills"]);
            if (build.contains("aquatic_skills"))
                bt.aquatic_skills = parseSkillBar(build["aquatic_skills"]);

            if (build.contains("pets")) {
                const auto& pets = build["pets"];
                if (pets.contains("terrestrial") && pets["terrestrial"].is_array()) {
                    int i = 0;
                    for (const auto& p : pets["terrestrial"]) {
                        if (i >= 2) break;
                        bt.pets.terrestrial[i] = p.is_null() ? 0 : p.get<uint32_t>();
                        i++;
                    }
                }
                if (pets.contains("aquatic") && pets["aquatic"].is_array()) {
                    int i = 0;
                    for (const auto& p : pets["aquatic"]) {
                        if (i >= 2) break;
                        bt.pets.aquatic[i] = p.is_null() ? 0 : p.get<uint32_t>();
                        i++;
                    }
                }
            }

            if (build.contains("legends") && build["legends"].is_array()) {
                int i = 0;
                for (const auto& leg : build["legends"]) {
                    if (i >= 2) break;
                    bt.legends.terrestrial[i] = leg.is_null() ? "" : leg.get<std::string>();
                    i++;
                }
            }
            if (build.contains("aquatic_legends") && build["aquatic_legends"].is_array()) {
                int i = 0;
                for (const auto& leg : build["aquatic_legends"]) {
                    if (i >= 2) break;
                    bt.legends.aquatic[i] = leg.is_null() ? "" : leg.get<std::string>();
                    i++;
                }
            }
        }

        return bt;
    }

    Character GW2API::ParseCharacterJson(const json& cj) {
        Character ch;
        ch.name = cj.value("name", "");
        ch.race = cj.value("race", "");
        ch.gender = cj.value("gender", "");
        ch.profession = cj.value("profession", "");
        ch.level = cj.value("level", 0);
        ch.age = cj.value("age", 0);
        ch.created = cj.value("created", "");
        ch.deaths = cj.value("deaths", 0);
        ch.active_build_tab = cj.value("active_build_tab", 0);
        ch.active_equipment_tab = cj.value("active_equipment_tab", 0);

        if (cj.contains("crafting") && cj["crafting"].is_array()) {
            std::vector<std::tuple<std::string, int, bool>> allCrafts;
            for (const auto& craft : cj["crafting"]) {
                int rating = craft.value("rating", 0);
                if (rating >= 1) {
                    allCrafts.emplace_back(
                        craft.value("discipline", ""),
                        rating,
                        craft.value("active", false));
                }
            }
            std::sort(allCrafts.begin(), allCrafts.end(),
                [](const auto& a, const auto& b) {
                    return std::get<2>(a) > std::get<2>(b);
                });
            for (const auto& [disc, lvl, active] : allCrafts) {
                ch.crafting.push_back(disc);
                ch.crafting_levels.push_back(lvl);
                ch.crafting_active.push_back(active);
            }
        }

        if (cj.contains("equipment") && cj["equipment"].is_array()) {
            for (const auto& eq_json : cj["equipment"]) {
                ch.equipment.push_back(ParseEquipmentItem(eq_json));
            }
        }

        if (cj.contains("build_tabs") && cj["build_tabs"].is_array()) {
            for (const auto& tab : cj["build_tabs"]) {
                ch.build_tabs.push_back(ParseBuildTemplate(tab));
            }
        }

        return ch;
    }

    // =========================================================================
    // Query helpers
    // =========================================================================

    FetchStatus GW2API::GetFetchStatus() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_fetch_status;
    }

    const std::string& GW2API::GetFetchStatusMessage() {
        return s_fetch_message;
    }

    std::string GW2API::GetCurrentFetchCharName() {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_fetch_status == FetchStatus::InProgress)
            return s_current_char.name;
        return "";
    }

    bool GW2API::HasCharacterData() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_has_character_data;
    }

    time_t GW2API::GetLastUpdated() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_last_updated;
    }

    const std::vector<Character>& GW2API::GetCharacters() {
        return s_characters;
    }

    const Character* GW2API::GetCharacter(const std::string& name) {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (const auto& c : s_characters) {
            if (c.name == name) return &c;
        }
        return nullptr;
    }

    const std::vector<std::string>& GW2API::GetPendingCharNames() {
        return s_pending_char_names;
    }

    // =========================================================================
    // Cache accessors
    // =========================================================================

    const ItemInfo* GW2API::GetItemInfo(uint32_t item_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_item_cache.find(item_id);
        return (it != s_item_cache.end()) ? &it->second : nullptr;
    }

    uint32_t GW2API::FindItemIdByName(const std::string& name) {
        if (name.empty()) return 0;
        std::lock_guard<std::mutex> lock(s_mutex);
        // Check full item cache first
        for (const auto& [id, info] : s_item_cache) {
            if (info.name == name) return id;
        }
        // Fallback to persistent name→ID cache
        auto it = s_item_name_id_cache.find(name);
        if (it != s_item_name_id_cache.end()) return it->second;
        return 0;
    }

    void GW2API::CacheItemNameId(const std::string& name, uint32_t id) {
        if (name.empty() || id == 0) return;
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_item_name_id_cache.find(name);
        if (it == s_item_name_id_cache.end() || it->second != id) {
            s_item_name_id_cache[name] = id;
            s_item_name_cache_dirty = true;
        }
    }

    bool GW2API::LoadItemNameCache() {
        std::string path = GetDataDirectory() + "/item_name_cache.json";
        std::ifstream file(path);
        if (!file.is_open()) return false;
        try {
            json j;
            file >> j;
            std::lock_guard<std::mutex> lock(s_mutex);
            s_item_name_id_cache.clear();
            if (j.is_object()) {
                for (auto it = j.begin(); it != j.end(); ++it) {
                    s_item_name_id_cache[it.key()] = it.value().get<uint32_t>();
                }
            }
            s_item_name_cache_dirty = false;
            if (s_api) {
                s_api->Log(LOGL_INFO, "AlterEgo",
                    ("Loaded item name cache: " + std::to_string(s_item_name_id_cache.size()) + " entries").c_str());
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    bool GW2API::SaveItemNameCache() {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_item_name_cache_dirty) return true;
        }
        EnsureDataDirectory();
        std::string path = GetDataDirectory() + "/item_name_cache.json";
        json j;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (const auto& [name, id] : s_item_name_id_cache) {
                j[name] = id;
            }
            s_item_name_cache_dirty = false;
        }
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump(2);
        file.flush();
        return true;
    }

    const SkinInfo* GW2API::GetSkinInfo(uint32_t skin_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_skin_cache.find(skin_id);
        return (it != s_skin_cache.end()) ? &it->second : nullptr;
    }

    const SpecializationInfo* GW2API::GetSpecInfo(uint32_t spec_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_spec_cache.find(spec_id);
        return (it != s_spec_cache.end()) ? &it->second : nullptr;
    }

    const TraitInfo* GW2API::GetTraitInfo(uint32_t trait_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_trait_cache.find(trait_id);
        return (it != s_trait_cache.end()) ? &it->second : nullptr;
    }

    const SkillInfo* GW2API::GetSkillInfo(uint32_t skill_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_skill_cache.find(skill_id);
        return (it != s_skill_cache.end()) ? &it->second : nullptr;
    }

    const DyeColor* GW2API::GetDyeColor(int color_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_dye_cache.find(color_id);
        return (it != s_dye_cache.end()) ? &it->second : nullptr;
    }

    const ItemStatInfo* GW2API::GetItemStatInfo(uint32_t stat_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_itemstat_cache.find(stat_id);
        return (it != s_itemstat_cache.end()) ? &it->second : nullptr;
    }

    const std::unordered_map<uint32_t, ItemStatInfo>& GW2API::GetAllItemStats() {
        return s_itemstat_cache;
    }

    void GW2API::FetchAllItemStatsAsync() {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_itemstat_cache.size() > 50) return; // Already loaded
        }
        std::thread([]() {
            std::string response = HttpGet("https://api.guildwars2.com/v2/itemstats?ids=all");
            if (response.empty()) return;
            try {
                json j = json::parse(response);
                if (j.is_array()) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    for (const auto& stat : j) {
                        if (!stat.contains("id")) continue;
                        uint32_t id = stat["id"].get<uint32_t>();
                        if (s_itemstat_cache.find(id) != s_itemstat_cache.end()) continue;
                        ItemStatInfo info;
                        info.id = id;
                        info.name = stat.value("name", "");
                        if (info.name.empty()) continue;
                        if (stat.contains("attributes") && stat["attributes"].is_array()) {
                            for (const auto& attr : stat["attributes"]) {
                                info.attributes.push_back(attr.value("attribute", ""));
                            }
                        }
                        s_itemstat_cache[id] = std::move(info);
                    }
                }
                if (s_api) {
                    std::string msg = "Loaded " + std::to_string(s_itemstat_cache.size()) + " stat combos";
                    s_api->Log(LOGL_INFO, "AlterEgo", msg.c_str());
                }
            } catch (...) {}
        }).detach();
    }

    const ProfessionInfo* GW2API::GetProfessionInfo(const std::string& profession) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_profession_cache.find(profession);
        return (it != s_profession_cache.end()) ? &it->second : nullptr;
    }

    const std::map<std::string, ProfessionWeaponData>* GW2API::GetProfessionWeapons(const std::string& profession) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_profession_weapons.find(profession);
        return (it != s_profession_weapons.end()) ? &it->second : nullptr;
    }

    void GW2API::FetchProfessionInfoAsync(const std::string& profession) {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_profession_cache.find(profession) != s_profession_cache.end()) return;
        }

        auto prof = profession;
        std::thread([prof]() {
            std::string url = "https://api.guildwars2.com/v2/professions/" + prof;
            std::string response = HttpGet(url);
            if (response.empty()) return;

            try {
                json j = json::parse(response);
                ProfessionInfo info;
                info.name = j.value("name", prof);
                info.icon_url = j.value("icon", "");
                info.icon_big_url = j.value("icon_big", "");

                std::lock_guard<std::mutex> lock(s_mutex);
                s_profession_cache[prof] = std::move(info);

                if (s_api) {
                    std::string msg = "Loaded profession info for " + prof;
                    s_api->Log(LOGL_INFO, "AlterEgo", msg.c_str());
                }
            } catch (...) {}
        }).detach();
    }

    // =========================================================================
    // Public endpoint fetching (direct HTTP, no auth needed)
    // =========================================================================

    std::string GW2API::HttpGet(const std::string& url) {
        HINTERNET hInternet = InternetOpenA("AlterEgo/1.0",
            INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return "";

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
                      INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
        if (!hUrl) {
            InternetCloseHandle(hInternet);
            return "";
        }

        std::string result;
        char buffer[8192];
        DWORD bytesRead;
        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            result.append(buffer, bytesRead);
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return result;
    }

    // --- Batch fetch: items ---

    void GW2API::FetchItemDetails(const std::vector<uint32_t>& item_ids) {
        if (item_ids.empty()) return;

        std::vector<uint32_t> to_fetch;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (uint32_t id : item_ids) {
                if (id != 0 && s_item_cache.find(id) == s_item_cache.end())
                    to_fetch.push_back(id);
            }
        }
        if (to_fetch.empty()) return;

        std::sort(to_fetch.begin(), to_fetch.end());
        to_fetch.erase(std::unique(to_fetch.begin(), to_fetch.end()), to_fetch.end());

        const size_t BATCH_SIZE = 200;
        for (size_t i = 0; i < to_fetch.size(); i += BATCH_SIZE) {
            std::string ids_param;
            size_t end = std::min(i + BATCH_SIZE, to_fetch.size());
            for (size_t j = i; j < end; j++) {
                if (!ids_param.empty()) ids_param += ",";
                ids_param += std::to_string(to_fetch[j]);
            }

            std::string url = "https://api.guildwars2.com/v2/items?ids=" + ids_param;
            std::string response = HttpGet(url);
            if (response.empty()) continue;

            try {
                json j = json::parse(response);
                if (j.is_array()) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    for (const auto& item : j) {
                        if (!item.contains("id")) continue;
                        ItemInfo info;
                        info.id = item["id"].get<uint32_t>();
                        info.name = item.value("name", "");
                        info.icon_url = item.value("icon", "");
                        info.rarity = item.value("rarity", "");
                        info.type = item.value("type", "");
                        info.chat_link = item.value("chat_link", "");
                        if (item.contains("description"))
                            info.description = StripHtmlTags(item["description"].get<std::string>());
                        if (item.contains("details"))
                            info.details = item["details"];
                        // Also populate persistent name→ID cache
                        if (!info.name.empty()) {
                            s_item_name_id_cache[info.name] = info.id;
                            s_item_name_cache_dirty = true;
                        }
                        s_item_cache[info.id] = info;
                    }
                }
            } catch (...) {}
        }
    }

    void GW2API::FetchSkinDetails(const std::vector<uint32_t>& skin_ids) {
        if (skin_ids.empty()) return;

        std::vector<uint32_t> to_fetch;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (uint32_t id : skin_ids) {
                if (id != 0 && s_skin_cache.find(id) == s_skin_cache.end())
                    to_fetch.push_back(id);
            }
        }
        if (to_fetch.empty()) return;

        std::sort(to_fetch.begin(), to_fetch.end());
        to_fetch.erase(std::unique(to_fetch.begin(), to_fetch.end()), to_fetch.end());

        const size_t BATCH_SIZE = 200;
        for (size_t i = 0; i < to_fetch.size(); i += BATCH_SIZE) {
            std::string ids_param;
            size_t end = std::min(i + BATCH_SIZE, to_fetch.size());
            for (size_t j = i; j < end; j++) {
                if (!ids_param.empty()) ids_param += ",";
                ids_param += std::to_string(to_fetch[j]);
            }

            std::string url = "https://api.guildwars2.com/v2/skins?ids=" + ids_param;
            std::string response = HttpGet(url);
            if (response.empty()) continue;

            try {
                json j = json::parse(response);
                if (j.is_array()) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    for (const auto& skin : j) {
                        if (!skin.contains("id")) continue;
                        SkinInfo info;
                        info.id = skin["id"].get<uint32_t>();
                        info.name = skin.value("name", "");
                        info.icon_url = skin.value("icon", "");
                        info.type = skin.value("type", "");
                        info.rarity = skin.value("rarity", "");
                        // Extract dye slot count from details.dye_slots.default
                        // Null entries = unused channels, only count non-null
                        if (skin.contains("details") && skin["details"].contains("dye_slots") &&
                            skin["details"]["dye_slots"].contains("default") &&
                            skin["details"]["dye_slots"]["default"].is_array()) {
                            int count = 0;
                            for (const auto& ds : skin["details"]["dye_slots"]["default"]) {
                                if (!ds.is_null()) count++;
                            }
                            info.dye_slot_count = count;
                        }
                        s_skin_cache[info.id] = info;
                    }
                }
            } catch (...) {}
        }
    }

    // --- Async wrappers ---

    void GW2API::FetchItemDetailsAsync(const std::vector<uint32_t>& item_ids) {
        auto ids = item_ids;
        std::thread([ids]() { FetchItemDetails(ids); }).detach();
    }

    void GW2API::FetchSkinDetailsAsync(const std::vector<uint32_t>& skin_ids) {
        auto ids = skin_ids;
        std::thread([ids]() { FetchSkinDetails(ids); }).detach();
    }

    void GW2API::FetchSpecDetailsAsync(const std::vector<uint32_t>& spec_ids) {
        auto ids = spec_ids;
        std::thread([ids]() {
            if (ids.empty()) return;
            std::vector<uint32_t> to_fetch;
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (uint32_t id : ids) {
                    if (id != 0 && s_spec_cache.find(id) == s_spec_cache.end())
                        to_fetch.push_back(id);
                }
            }
            if (to_fetch.empty()) return;

            std::sort(to_fetch.begin(), to_fetch.end());
            to_fetch.erase(std::unique(to_fetch.begin(), to_fetch.end()), to_fetch.end());

            std::string ids_param;
            for (size_t i = 0; i < to_fetch.size(); i++) {
                if (i > 0) ids_param += ",";
                ids_param += std::to_string(to_fetch[i]);
            }

            std::string url = "https://api.guildwars2.com/v2/specializations?ids=" + ids_param;
            std::string response = HttpGet(url);
            if (response.empty()) return;

            try {
                json j = json::parse(response);
                if (j.is_array()) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    for (const auto& spec : j) {
                        if (!spec.contains("id")) continue;
                        SpecializationInfo info;
                        info.id = spec["id"].get<uint32_t>();
                        info.name = spec.value("name", "");
                        info.icon_url = spec.value("icon", "");
                        info.background_url = spec.value("background", "");
                        info.profession = spec.value("profession", "");
                        info.profession_icon_url = spec.value("profession_icon", "");
                        info.profession_icon_big_url = spec.value("profession_icon_big", "");
                        info.elite = spec.value("elite", false);
                        if (spec.contains("minor_traits")) {
                            for (const auto& t : spec["minor_traits"])
                                info.minor_traits.push_back(t.get<uint32_t>());
                        }
                        if (spec.contains("major_traits")) {
                            for (const auto& t : spec["major_traits"])
                                info.major_traits.push_back(t.get<uint32_t>());
                        }
                        s_spec_cache[info.id] = info;
                    }
                }
            } catch (...) {}

            // Chain-fetch all trait details for loaded specs
            std::vector<uint32_t> all_trait_ids;
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (uint32_t sid : to_fetch) {
                    auto it = s_spec_cache.find(sid);
                    if (it == s_spec_cache.end()) continue;
                    for (uint32_t tid : it->second.minor_traits) all_trait_ids.push_back(tid);
                    for (uint32_t tid : it->second.major_traits) all_trait_ids.push_back(tid);
                }
            }
            if (!all_trait_ids.empty()) {
                FetchTraitDetails(all_trait_ids);
            }
        }).detach();
    }

    void GW2API::FetchTraitDetails(const std::vector<uint32_t>& trait_ids) {
        if (trait_ids.empty()) return;
        std::vector<uint32_t> to_fetch;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (uint32_t id : trait_ids) {
                if (id != 0 && s_trait_cache.find(id) == s_trait_cache.end())
                    to_fetch.push_back(id);
            }
        }
        if (to_fetch.empty()) return;

        std::sort(to_fetch.begin(), to_fetch.end());
        to_fetch.erase(std::unique(to_fetch.begin(), to_fetch.end()), to_fetch.end());

        const size_t BATCH_SIZE = 200;
        for (size_t i = 0; i < to_fetch.size(); i += BATCH_SIZE) {
            std::string ids_param;
            size_t end = std::min(i + BATCH_SIZE, to_fetch.size());
            for (size_t j = i; j < end; j++) {
                if (!ids_param.empty()) ids_param += ",";
                ids_param += std::to_string(to_fetch[j]);
            }

            std::string response = HttpGet("https://api.guildwars2.com/v2/traits?ids=" + ids_param);
            if (response.empty()) continue;

            try {
                json j = json::parse(response);
                if (j.is_array()) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    for (const auto& trait : j) {
                        if (!trait.contains("id")) continue;
                        TraitInfo info;
                        info.id = trait["id"].get<uint32_t>();
                        info.name = trait.value("name", "");
                        info.icon_url = trait.value("icon", "");
                        if (trait.contains("description"))
                            info.description = StripHtmlTags(trait["description"].get<std::string>());
                        info.tier = trait.value("tier", 0);
                        info.order = trait.value("order", 0);
                        info.slot = trait.value("slot", "");
                        if (trait.contains("facts"))
                            info.facts = trait["facts"];
                        s_trait_cache[info.id] = info;
                    }
                }
            } catch (...) {}
        }
    }

    void GW2API::FetchTraitDetailsAsync(const std::vector<uint32_t>& trait_ids) {
        auto ids = trait_ids;
        std::thread([ids]() { FetchTraitDetails(ids); }).detach();
    }

    void GW2API::FetchSkillDetailsAsync(const std::vector<uint32_t>& skill_ids) {
        auto ids = skill_ids;
        std::thread([ids]() {
            if (ids.empty()) return;
            std::vector<uint32_t> to_fetch;
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (uint32_t id : ids) {
                    if (id != 0 && s_skill_cache.find(id) == s_skill_cache.end())
                        to_fetch.push_back(id);
                }
            }
            if (to_fetch.empty()) return;

            std::sort(to_fetch.begin(), to_fetch.end());
            to_fetch.erase(std::unique(to_fetch.begin(), to_fetch.end()), to_fetch.end());

            const size_t BATCH_SIZE = 200;
            for (size_t i = 0; i < to_fetch.size(); i += BATCH_SIZE) {
                std::string ids_param;
                size_t end = std::min(i + BATCH_SIZE, to_fetch.size());
                for (size_t j = i; j < end; j++) {
                    if (!ids_param.empty()) ids_param += ",";
                    ids_param += std::to_string(to_fetch[j]);
                }

                std::string response = HttpGet("https://api.guildwars2.com/v2/skills?ids=" + ids_param);
                if (response.empty()) continue;

                try {
                    json j = json::parse(response);
                    if (j.is_array()) {
                        std::lock_guard<std::mutex> lock(s_mutex);
                        for (const auto& skill : j) {
                            if (!skill.contains("id")) continue;
                            SkillInfo info;
                            info.id = skill["id"].get<uint32_t>();
                            info.name = skill.value("name", "");
                            info.icon_url = skill.value("icon", "");
                            if (skill.contains("description"))
                                info.description = StripHtmlTags(skill["description"].get<std::string>());
                            info.type = skill.value("type", "");
                            if (skill.contains("facts"))
                                info.facts = skill["facts"];
                            s_skill_cache[info.id] = info;
                        }
                    }
                } catch (...) {}
            }
        }).detach();
    }

    void GW2API::FetchItemStatDetailsAsync(const std::vector<uint32_t>& stat_ids) {
        auto ids = stat_ids;
        std::thread([ids]() {
            if (ids.empty()) return;
            std::vector<uint32_t> to_fetch;
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (uint32_t id : ids) {
                    if (id != 0 && s_itemstat_cache.find(id) == s_itemstat_cache.end())
                        to_fetch.push_back(id);
                }
            }
            if (to_fetch.empty()) return;

            std::sort(to_fetch.begin(), to_fetch.end());
            to_fetch.erase(std::unique(to_fetch.begin(), to_fetch.end()), to_fetch.end());

            const size_t BATCH_SIZE = 200;
            for (size_t i = 0; i < to_fetch.size(); i += BATCH_SIZE) {
                std::string ids_param;
                size_t end = std::min(i + BATCH_SIZE, to_fetch.size());
                for (size_t j = i; j < end; j++) {
                    if (!ids_param.empty()) ids_param += ",";
                    ids_param += std::to_string(to_fetch[j]);
                }

                std::string response = HttpGet("https://api.guildwars2.com/v2/itemstats?ids=" + ids_param);
                if (response.empty()) continue;

                try {
                    json j = json::parse(response);
                    if (j.is_array()) {
                        std::lock_guard<std::mutex> lock(s_mutex);
                        for (const auto& stat : j) {
                            if (!stat.contains("id")) continue;
                            ItemStatInfo info;
                            info.id = stat["id"].get<uint32_t>();
                            info.name = stat.value("name", "");
                            if (stat.contains("attributes") && stat["attributes"].is_array()) {
                                for (const auto& attr : stat["attributes"]) {
                                    info.attributes.push_back(attr.value("attribute", ""));
                                }
                            }
                            s_itemstat_cache[info.id] = info;
                        }
                    }
                } catch (...) {}
            }
        }).detach();
    }

    void GW2API::FetchDyeColorsAsync() {
        std::thread([]() {
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                if (!s_dye_cache.empty()) return;
            }

            std::string response = HttpGet("https://api.guildwars2.com/v2/colors?ids=all");
            if (response.empty()) return;

            try {
                json j = json::parse(response);
                if (j.is_array()) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    for (const auto& color : j) {
                        if (!color.contains("id")) continue;
                        DyeColor dc;
                        dc.id = color["id"].get<int>();
                        dc.name = color.value("name", "");
                        if (color.contains("base_rgb") && color["base_rgb"].is_array() && color["base_rgb"].size() >= 3) {
                            dc.r = color["base_rgb"][0].get<int>();
                            dc.g = color["base_rgb"][1].get<int>();
                            dc.b = color["base_rgb"][2].get<int>();
                        }
                        s_dye_cache[dc.id] = dc;
                    }
                }
            } catch (...) {}
        }).detach();
    }

    // =========================================================================
    // Persistence
    // =========================================================================

    bool GW2API::SaveCharacterData() {
        EnsureDataDirectory();
        std::string path = GetDataDirectory() + "/characters.json";

        json j;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            j["last_updated"] = (int64_t)s_last_updated;
            json chars_arr = json::array();
            for (const auto& ch : s_characters) {
                json cj;
                cj["name"] = ch.name;
                cj["race"] = ch.race;
                cj["gender"] = ch.gender;
                cj["profession"] = ch.profession;
                cj["level"] = ch.level;
                cj["age"] = ch.age;
                cj["created"] = ch.created;
                cj["last_modified"] = ch.last_modified;
                cj["deaths"] = ch.deaths;
                if (!ch.crafting.empty()) {
                    json craft_arr = json::array();
                    for (size_t ci = 0; ci < ch.crafting.size(); ci++) {
                        json craft;
                        craft["discipline"] = ch.crafting[ci];
                        craft["rating"] = (ci < ch.crafting_levels.size()) ? ch.crafting_levels[ci] : 0;
                        craft["active"] = (ci < ch.crafting_active.size()) ? ch.crafting_active[ci] : true;
                        craft_arr.push_back(craft);
                    }
                    cj["crafting"] = craft_arr;
                }
                cj["active_build_tab"] = ch.active_build_tab;
                cj["active_equipment_tab"] = ch.active_equipment_tab;

                json eq_arr = json::array();
                for (const auto& eq : ch.equipment) {
                    json ej;
                    ej["id"] = eq.id;
                    ej["slot"] = eq.slot;
                    ej["skin"] = eq.skin;
                    ej["tab"] = eq.tab;
                    if (!eq.upgrades.empty()) ej["upgrades"] = eq.upgrades;
                    if (!eq.infusions.empty()) ej["infusions"] = eq.infusions;
                    if (!eq.dyes.empty()) ej["dyes"] = eq.dyes;
                    if (eq.stat_id != 0) ej["stat_id"] = eq.stat_id;
                    if (!eq.attributes.empty()) {
                        json attrs;
                        for (const auto& [k, v] : eq.attributes) attrs[k] = v;
                        ej["attributes"] = attrs;
                    }
                    eq_arr.push_back(ej);
                }
                cj["equipment"] = eq_arr;

                if (!ch.equipment_tab_names.empty()) {
                    json etn;
                    for (const auto& [tab, name] : ch.equipment_tab_names) {
                        etn[std::to_string(tab)] = name;
                    }
                    cj["equipment_tab_names"] = etn;
                }

                json bt_arr = json::array();
                for (const auto& bt : ch.build_tabs) {
                    json bj;
                    bj["tab"] = bt.tab;
                    bj["is_active"] = bt.is_active;
                    bj["name"] = bt.name;
                    bj["profession"] = bt.profession;

                    json specs = json::array();
                    for (int i = 0; i < 3; i++) {
                        json sj;
                        sj["id"] = bt.specializations[i].spec_id;
                        sj["traits"] = {bt.specializations[i].traits[0],
                                        bt.specializations[i].traits[1],
                                        bt.specializations[i].traits[2]};
                        specs.push_back(sj);
                    }
                    bj["specializations"] = specs;

                    auto skillBarToJson = [](const SkillBar& sb) -> json {
                        json sj;
                        sj["heal"] = sb.heal;
                        sj["utilities"] = {sb.utilities[0], sb.utilities[1], sb.utilities[2]};
                        sj["elite"] = sb.elite;
                        return sj;
                    };
                    bj["skills"] = skillBarToJson(bt.terrestrial_skills);
                    bj["aquatic_skills"] = skillBarToJson(bt.aquatic_skills);

                    bt_arr.push_back(bj);
                }
                cj["build_tabs"] = bt_arr;

                chars_arr.push_back(cj);
            }
            j["characters"] = chars_arr;
        }

        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump(2);
        file.flush();
        return true;
    }

    bool GW2API::LoadCharacterData() {
        std::string path = GetDataDirectory() + "/characters.json";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j;

            std::lock_guard<std::mutex> lock(s_mutex);
            s_last_updated = j.value("last_updated", (int64_t)0);

            if (j.contains("characters") && j["characters"].is_array()) {
                s_characters.clear();
                for (const auto& cj : j["characters"]) {
                    Character ch;
                    ch.name = cj.value("name", "");
                    ch.race = cj.value("race", "");
                    ch.gender = cj.value("gender", "");
                    ch.profession = cj.value("profession", "");
                    ch.level = cj.value("level", 0);
                    ch.age = cj.value("age", 0);
                    ch.created = cj.value("created", "");
                    ch.last_modified = cj.value("last_modified", "");
                    ch.deaths = cj.value("deaths", 0);
                    if (cj.contains("crafting") && cj["crafting"].is_array()) {
                        for (const auto& craft : cj["crafting"]) {
                            ch.crafting.push_back(craft.value("discipline", ""));
                            ch.crafting_levels.push_back(craft.value("rating", 0));
                            ch.crafting_active.push_back(craft.value("active", true));
                        }
                    }
                    ch.active_build_tab = cj.value("active_build_tab", 0);
                    ch.active_equipment_tab = cj.value("active_equipment_tab", 0);

                    if (cj.contains("equipment_tab_names") && cj["equipment_tab_names"].is_object()) {
                        for (auto it = cj["equipment_tab_names"].begin(); it != cj["equipment_tab_names"].end(); ++it) {
                            ch.equipment_tab_names[std::stoi(it.key())] = it.value().get<std::string>();
                        }
                    }

                    if (cj.contains("equipment") && cj["equipment"].is_array()) {
                        for (const auto& ej : cj["equipment"]) {
                            EquipmentItem eq;
                            eq.id = ej.value("id", (uint32_t)0);
                            eq.slot = ej.value("slot", "");
                            eq.skin = ej.value("skin", (uint32_t)0);
                            eq.tab = ej.value("tab", 0);
                            if (ej.contains("upgrades")) {
                                for (const auto& u : ej["upgrades"])
                                    eq.upgrades.push_back(u.get<uint32_t>());
                            }
                            if (ej.contains("infusions")) {
                                for (const auto& inf : ej["infusions"])
                                    eq.infusions.push_back(inf.get<uint32_t>());
                            }
                            if (ej.contains("dyes")) {
                                for (const auto& d : ej["dyes"])
                                    eq.dyes.push_back(d.get<int>());
                            }
                            eq.stat_id = ej.value("stat_id", (uint32_t)0);
                            if (ej.contains("attributes") && ej["attributes"].is_object()) {
                                for (auto it = ej["attributes"].begin(); it != ej["attributes"].end(); ++it) {
                                    eq.attributes[it.key()] = it.value().get<int>();
                                }
                            }
                            ch.equipment.push_back(eq);
                        }
                    }

                    if (cj.contains("build_tabs") && cj["build_tabs"].is_array()) {
                        for (const auto& bj : cj["build_tabs"]) {
                            BuildTemplate bt;
                            bt.tab = bj.value("tab", 0);
                            bt.is_active = bj.value("is_active", false);
                            bt.name = bj.value("name", "");
                            bt.profession = bj.value("profession", "");

                            if (bj.contains("specializations") && bj["specializations"].is_array()) {
                                int idx = 0;
                                for (const auto& sj : bj["specializations"]) {
                                    if (idx >= 3) break;
                                    bt.specializations[idx].spec_id = sj.value("id", (uint32_t)0);
                                    if (sj.contains("traits") && sj["traits"].is_array()) {
                                        int ti = 0;
                                        for (const auto& t : sj["traits"]) {
                                            if (ti >= 3) break;
                                            bt.specializations[idx].traits[ti] = t.get<int>();
                                            ti++;
                                        }
                                    }
                                    idx++;
                                }
                            }

                            auto parseSkillBarJson = [](const json& sj) -> SkillBar {
                                SkillBar sb;
                                sb.heal = sj.value("heal", (uint32_t)0);
                                if (sj.contains("utilities") && sj["utilities"].is_array()) {
                                    int i = 0;
                                    for (const auto& u : sj["utilities"]) {
                                        if (i >= 3) break;
                                        sb.utilities[i] = u.get<uint32_t>();
                                        i++;
                                    }
                                }
                                sb.elite = sj.value("elite", (uint32_t)0);
                                return sb;
                            };

                            if (bj.contains("skills"))
                                bt.terrestrial_skills = parseSkillBarJson(bj["skills"]);
                            if (bj.contains("aquatic_skills"))
                                bt.aquatic_skills = parseSkillBarJson(bj["aquatic_skills"]);

                            ch.build_tabs.push_back(bt);
                        }
                    }

                    s_characters.push_back(ch);
                }
                s_has_character_data = !s_characters.empty();
            }

            return true;
        } catch (...) {
            return false;
        }
    }

    // =========================================================================
    // Build Library
    // =========================================================================

    static const char* GameModeToStr(GameMode m) {
        switch (m) {
            case GameMode::PvE:     return "PvE";
            case GameMode::WvW:     return "WvW";
            case GameMode::PvP:     return "PvP";
            case GameMode::Raid:    return "Raid";
            case GameMode::Fractal: return "Fractal";
            default:                return "Other";
        }
    }

    static GameMode StrToGameMode(const std::string& s) {
        if (s == "PvE")     return GameMode::PvE;
        if (s == "WvW")     return GameMode::WvW;
        if (s == "PvP")     return GameMode::PvP;
        if (s == "Raid")    return GameMode::Raid;
        if (s == "Fractal") return GameMode::Fractal;
        return GameMode::Other;
    }

    const std::vector<SavedBuild>& GW2API::GetSavedBuilds() {
        return s_saved_builds;
    }

    bool GW2API::AddSavedBuild(SavedBuild build) {
        if (build.id.empty()) {
            build.id = std::to_string(std::time(nullptr)) + "_" +
                       std::to_string(s_saved_builds.size());
        }
        if (build.created == 0) build.created = std::time(nullptr);

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_saved_builds.push_back(std::move(build));
        }
        return SaveBuildLibrary();
    }

    bool GW2API::RemoveSavedBuild(const std::string& id) {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            auto it = std::remove_if(s_saved_builds.begin(), s_saved_builds.end(),
                [&](const SavedBuild& b) { return b.id == id; });
            if (it == s_saved_builds.end()) return false;
            s_saved_builds.erase(it, s_saved_builds.end());
        }
        return SaveBuildLibrary();
    }

    bool GW2API::UpdateSavedBuild(const std::string& id, const std::string& name, const std::string& notes) {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (auto& b : s_saved_builds) {
                if (b.id == id) {
                    b.name = name;
                    b.notes = notes;
                    break;
                }
            }
        }
        return SaveBuildLibrary();
    }

    bool GW2API::ReorderSavedBuild(int fromIdx, int toIdx) {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            int n = (int)s_saved_builds.size();
            if (fromIdx < 0 || fromIdx >= n || toIdx < 0 || toIdx > n) return false;
            SavedBuild moved = std::move(s_saved_builds[fromIdx]);
            s_saved_builds.erase(s_saved_builds.begin() + fromIdx);
            int insertAt = (fromIdx < toIdx) ? toIdx - 1 : toIdx;
            if (insertAt > (int)s_saved_builds.size()) insertAt = (int)s_saved_builds.size();
            s_saved_builds.insert(s_saved_builds.begin() + insertAt, std::move(moved));
        }
        return SaveBuildLibrary();
    }

    bool GW2API::SaveBuildLibrary() {
        EnsureDataDirectory();
        std::string path = GetDataDirectory() + "/build_library.json";

        json j;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            json arr = json::array();
            for (const auto& b : s_saved_builds) {
                json bj;
                bj["id"] = b.id;
                bj["name"] = b.name;
                bj["chat_link"] = b.chat_link;
                bj["profession"] = b.profession;
                bj["game_mode"] = GameModeToStr(b.game_mode);
                bj["notes"] = b.notes;
                bj["created"] = (int64_t)b.created;

                json specs = json::array();
                for (int i = 0; i < 3; i++) {
                    json sj;
                    sj["id"] = b.specializations[i].spec_id;
                    sj["traits"] = {b.specializations[i].traits[0],
                                    b.specializations[i].traits[1],
                                    b.specializations[i].traits[2]};
                    specs.push_back(sj);
                }
                bj["specializations"] = specs;

                auto skillBarToJson = [](const SkillBar& sb) -> json {
                    json sj;
                    sj["heal"] = sb.heal;
                    sj["utilities"] = {sb.utilities[0], sb.utilities[1], sb.utilities[2]};
                    sj["elite"] = sb.elite;
                    return sj;
                };
                bj["skills"] = skillBarToJson(b.terrestrial_skills);
                bj["aquatic_skills"] = skillBarToJson(b.aquatic_skills);

                bj["pets"] = {b.pets.terrestrial[0], b.pets.terrestrial[1],
                              b.pets.aquatic[0], b.pets.aquatic[1]};
                bj["legend_codes"] = {b.legend_codes[0], b.legend_codes[1],
                                      b.legend_codes[2], b.legend_codes[3]};
                if (!b.weapons.empty()) bj["weapons"] = b.weapons;

                // Gear data
                if (!b.gear.empty()) {
                    json gearJ = json::object();
                    for (const auto& [slot, gs] : b.gear) {
                        json gsj;
                        gsj["stat_id"] = gs.stat_id;
                        gsj["stat_name"] = gs.stat_name;
                        if (!gs.rune.empty()) gsj["rune"] = gs.rune;
                        if (gs.rune_id != 0) gsj["rune_id"] = gs.rune_id;
                        if (!gs.sigil.empty()) gsj["sigil"] = gs.sigil;
                        if (gs.sigil_id != 0) gsj["sigil_id"] = gs.sigil_id;
                        if (!gs.infusion.empty()) gsj["infusion"] = gs.infusion;
                        if (!gs.weapon_type.empty()) gsj["weapon_type"] = gs.weapon_type;
                        gearJ[slot] = gsj;
                    }
                    bj["gear"] = gearJ;
                }
                if (!b.rune_name.empty()) bj["rune_name"] = b.rune_name;
                if (b.rune_id != 0) bj["rune_id"] = b.rune_id;
                if (!b.relic_name.empty()) bj["relic_name"] = b.relic_name;
                if (b.relic_id != 0) bj["relic_id"] = b.relic_id;

                arr.push_back(bj);
            }
            j["builds"] = arr;
        }

        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump(2);
        file.flush();
        return true;
    }

    bool GW2API::LoadBuildLibrary() {
        std::string path = GetDataDirectory() + "/build_library.json";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j;

            std::lock_guard<std::mutex> lock(s_mutex);
            s_saved_builds.clear();

            if (j.contains("builds") && j["builds"].is_array()) {
                for (const auto& bj : j["builds"]) {
                    SavedBuild b;
                    b.id = bj.value("id", "");
                    b.name = bj.value("name", "");
                    b.chat_link = bj.value("chat_link", "");
                    b.profession = bj.value("profession", "");
                    b.game_mode = StrToGameMode(bj.value("game_mode", "PvE"));
                    b.notes = bj.value("notes", "");
                    b.created = bj.value("created", (int64_t)0);

                    if (bj.contains("specializations") && bj["specializations"].is_array()) {
                        int idx = 0;
                        for (const auto& sj : bj["specializations"]) {
                            if (idx >= 3) break;
                            b.specializations[idx].spec_id = sj.value("id", (uint32_t)0);
                            if (sj.contains("traits") && sj["traits"].is_array()) {
                                int ti = 0;
                                for (const auto& t : sj["traits"]) {
                                    if (ti >= 3) break;
                                    b.specializations[idx].traits[ti] = t.get<int>();
                                    ti++;
                                }
                            }
                            idx++;
                        }
                    }

                    auto parseSkillBarJson = [](const json& sj) -> SkillBar {
                        SkillBar sb;
                        sb.heal = sj.value("heal", (uint32_t)0);
                        if (sj.contains("utilities") && sj["utilities"].is_array()) {
                            int i = 0;
                            for (const auto& u : sj["utilities"]) {
                                if (i >= 3) break;
                                sb.utilities[i] = u.get<uint32_t>();
                                i++;
                            }
                        }
                        sb.elite = sj.value("elite", (uint32_t)0);
                        return sb;
                    };

                    if (bj.contains("skills"))
                        b.terrestrial_skills = parseSkillBarJson(bj["skills"]);
                    if (bj.contains("aquatic_skills"))
                        b.aquatic_skills = parseSkillBarJson(bj["aquatic_skills"]);

                    if (bj.contains("pets") && bj["pets"].is_array() && bj["pets"].size() >= 4) {
                        b.pets.terrestrial[0] = bj["pets"][0].get<uint32_t>();
                        b.pets.terrestrial[1] = bj["pets"][1].get<uint32_t>();
                        b.pets.aquatic[0] = bj["pets"][2].get<uint32_t>();
                        b.pets.aquatic[1] = bj["pets"][3].get<uint32_t>();
                    }
                    if (bj.contains("legend_codes") && bj["legend_codes"].is_array() && bj["legend_codes"].size() >= 4) {
                        for (int i = 0; i < 4; i++)
                            b.legend_codes[i] = bj["legend_codes"][i].get<uint8_t>();
                    }
                    if (bj.contains("weapons") && bj["weapons"].is_array()) {
                        for (const auto& w : bj["weapons"])
                            b.weapons.push_back(w.get<uint32_t>());
                    }

                    // Gear data
                    if (bj.contains("gear") && bj["gear"].is_object()) {
                        for (auto it = bj["gear"].begin(); it != bj["gear"].end(); ++it) {
                            BuildGearSlot gs;
                            gs.slot = it.key();
                            gs.stat_id = it.value().value("stat_id", (uint32_t)0);
                            gs.stat_name = it.value().value("stat_name", "");
                            gs.rune = it.value().value("rune", "");
                            gs.rune_id = it.value().value("rune_id", (uint32_t)0);
                            gs.sigil = it.value().value("sigil", "");
                            gs.sigil_id = it.value().value("sigil_id", (uint32_t)0);
                            gs.infusion = it.value().value("infusion", "");
                            gs.weapon_type = it.value().value("weapon_type", "");
                            b.gear[gs.slot] = gs;
                        }
                    }
                    b.rune_name = bj.value("rune_name", "");
                    b.rune_id = bj.value("rune_id", (uint32_t)0);
                    b.relic_name = bj.value("relic_name", "");
                    b.relic_id = bj.value("relic_id", (uint32_t)0);

                    s_saved_builds.push_back(std::move(b));
                }
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    // =========================================================================
    // Palette ID Mapping
    // =========================================================================

    static const std::unordered_map<std::string, uint8_t> PROFESSION_CODES = {
        {"Guardian", 1}, {"Warrior", 2}, {"Engineer", 3}, {"Ranger", 4},
        {"Thief", 5}, {"Elementalist", 6}, {"Mesmer", 7}, {"Necromancer", 8},
        {"Revenant", 9}
    };

    static std::string ProfessionFromCode(uint8_t code) {
        for (const auto& [name, c] : PROFESSION_CODES) {
            if (c == code) return name;
        }
        return "";
    }

    static const std::unordered_map<std::string, uint8_t> LEGEND_NAME_TO_CODE = {
        {"Legend1", 1}, {"Legend2", 2}, {"Legend3", 3}, {"Legend4", 4},
        {"Legend5", 5}, {"Legend6", 6}, {"Legend7", 7}, {"Legend8", 8}
    };

    bool GW2API::HasPaletteData(const std::string& profession) {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_palette_to_skill.find(profession) != s_palette_to_skill.end();
    }

    uint32_t GW2API::GetSkillIdFromPalette(const std::string& profession, uint16_t palette_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto pit = s_palette_to_skill.find(profession);
        if (pit == s_palette_to_skill.end()) return 0;
        auto it = pit->second.find(palette_id);
        return (it != pit->second.end()) ? it->second : 0;
    }

    uint16_t GW2API::GetPaletteIdFromSkill(const std::string& profession, uint32_t skill_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto pit = s_skill_to_palette.find(profession);
        if (pit == s_skill_to_palette.end()) return 0;
        auto it = pit->second.find(skill_id);
        return (it != pit->second.end()) ? it->second : 0;
    }

    void GW2API::FetchProfessionPaletteAsync(const std::string& profession) {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            // Already have data or already fetching
            if (s_palette_to_skill.find(profession) != s_palette_to_skill.end()) return;
            if (s_palette_fetching.count(profession)) return;
            s_palette_fetching.insert(profession);
        }

        auto prof = profession;
        std::thread([prof]() {
            // Fetch with v=latest to get skills_by_palette
            std::string url = "https://api.guildwars2.com/v2/professions/" + prof + "?v=latest";
            std::string response = HttpGet(url);
            if (response.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_palette_fetching.erase(prof);
                return;
            }

            try {
                json j = json::parse(response);
                if (j.contains("skills_by_palette") && j["skills_by_palette"].is_array()) {
                    std::unordered_map<uint16_t, uint32_t> p2s;
                    std::unordered_map<uint32_t, uint16_t> s2p;
                    for (const auto& pair : j["skills_by_palette"]) {
                        if (!pair.is_array() || pair.size() < 2) continue;
                        uint16_t palette_id = pair[0].get<uint16_t>();
                        uint32_t skill_id = pair[1].get<uint32_t>();
                        p2s[palette_id] = skill_id;
                        s2p[skill_id] = palette_id;
                    }

                    // Parse weapon data
                    std::map<std::string, ProfessionWeaponData> weapons;
                    if (j.contains("weapons") && j["weapons"].is_object()) {
                        for (auto it = j["weapons"].begin(); it != j["weapons"].end(); ++it) {
                            ProfessionWeaponData wd;
                            const auto& wj = it.value();
                            if (wj.contains("flags") && wj["flags"].is_array()) {
                                for (const auto& f : wj["flags"]) {
                                    std::string flag = f.get<std::string>();
                                    if (flag == "Mainhand") wd.mainhand = true;
                                    else if (flag == "Offhand") wd.offhand = true;
                                    else if (flag == "TwoHand") wd.two_handed = true;
                                }
                            }
                            wd.specialization = wj.value("specialization", (uint32_t)0);
                            if (wj.contains("skills") && wj["skills"].is_array()) {
                                for (const auto& sk : wj["skills"]) {
                                    std::string slot = sk.value("slot", "");
                                    uint32_t sid = sk.value("id", (uint32_t)0);
                                    std::string offhand = sk.value("offhand", "");
                                    std::string attunement = sk.value("attunement", "");

                                    int slotIdx = -1;
                                    if (slot == "Weapon_1") slotIdx = 0;
                                    else if (slot == "Weapon_2") slotIdx = 1;
                                    else if (slot == "Weapon_3") slotIdx = 2;
                                    else if (slot == "Weapon_4") slotIdx = 3;
                                    else if (slot == "Weapon_5") slotIdx = 4;
                                    if (slotIdx < 0) continue;

                                    if (!attunement.empty()) {
                                        // Elementalist: store per-attunement
                                        wd.attunement_skills[attunement][slotIdx] = sid;
                                    } else if (!offhand.empty() && slotIdx == 2) {
                                        // Thief dual wield: Weapon_3 varies by offhand
                                        wd.dual_wield[offhand] = sid;
                                    } else {
                                        // Default (no qualifier)
                                        wd.skills[slotIdx] = sid;
                                    }
                                }
                            }
                            weapons[it.key()] = wd;
                        }
                    }

                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_palette_to_skill[prof] = std::move(p2s);
                    s_skill_to_palette[prof] = std::move(s2p);
                    if (!weapons.empty()) s_profession_weapons[prof] = std::move(weapons);
                    s_palette_fetching.erase(prof);

                    if (s_api) {
                        std::string msg = "Loaded palette mapping for " + prof +
                            " (" + std::to_string(s_palette_to_skill[prof].size()) + " skills)";
                        s_api->Log(LOGL_INFO, "AlterEgo", msg.c_str());
                    }
                }
            } catch (...) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_palette_fetching.erase(prof);
            }
        }).detach();
    }

} // namespace AlterEgo
