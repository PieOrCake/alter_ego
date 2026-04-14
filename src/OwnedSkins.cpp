#include "OwnedSkins.h"
#include "HoardAndSeekAPI.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

namespace Skinventory {

    AddonAPI_t* OwnedSkins::s_API = nullptr;
    std::string OwnedSkins::s_data_dir;
    std::unordered_set<uint32_t> OwnedSkins::s_owned_skins;
    std::unordered_set<uint32_t> OwnedSkins::s_queried_skins;
    std::mutex OwnedSkins::s_mutex;
    std::atomic<bool> OwnedSkins::s_has_data{false};
    std::atomic<bool> OwnedSkins::s_hs_available{false};
    std::atomic<bool> OwnedSkins::s_querying{false};
    std::atomic<bool> OwnedSkins::s_data_updated{false};
    std::string OwnedSkins::s_status_message = "Waiting for Hoard & Seek...";
    std::atomic<uint64_t> OwnedSkins::s_generation{0};
    std::unordered_set<uint32_t> OwnedSkins::s_locally_marked;
    std::vector<uint32_t> OwnedSkins::s_pending_ids;
    size_t OwnedSkins::s_batch_index = 0;
    std::atomic<bool> OwnedSkins::s_batch_pending{false};
    std::string OwnedSkins::s_account_name;

    #define SKINVENTORY_REQUESTER "Skinventory"
    #define EV_SKINVENTORY_SKINS_RESPONSE "EV_SKINVENTORY_SKINS_RESPONSE"
    #define EV_SKINVENTORY_API_RESPONSE "EV_SKINVENTORY_API_RESPONSE"

    void OwnedSkins::Tick() {
        if (s_batch_pending.exchange(false)) {
            SendNextBatch();
        }
    }

    void OwnedSkins::SetDataDirectory(const std::string& dir) {
        s_data_dir = dir;
    }

    void OwnedSkins::Initialize(AddonAPI_t* api) {
        s_API = api;
        if (!s_API) return;

        // Load cached owned skins from disk for instant UI
        if (LoadFromCache()) {
            s_has_data = true;
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Owned: " + std::to_string(s_owned_skins.size()) + " (cached)";
        }

        s_API->Events_Subscribe(EV_HOARD_PONG, OnPong);
        s_API->Events_Subscribe(EV_HOARD_DATA_UPDATED, OnDataUpdated);
        s_API->Events_Subscribe(EV_SKINVENTORY_SKINS_RESPONSE, OnSkinsResponse);
        s_API->Events_Subscribe(EV_SKINVENTORY_API_RESPONSE, OnApiResponse);
    }

    void OwnedSkins::Shutdown() {
        if (!s_API) return;

        s_API->Events_Unsubscribe(EV_HOARD_PONG, OnPong);
        s_API->Events_Unsubscribe(EV_HOARD_DATA_UPDATED, OnDataUpdated);
        s_API->Events_Unsubscribe(EV_SKINVENTORY_SKINS_RESPONSE, OnSkinsResponse);
        s_API->Events_Unsubscribe(EV_SKINVENTORY_API_RESPONSE, OnApiResponse);
        s_API = nullptr;
    }

    void OwnedSkins::PingHoardAndSeek() {
        if (!s_API) return;
        s_API->Events_Raise(EV_HOARD_PING, nullptr);
    }

    void OwnedSkins::SetAccountName(const std::string& name) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_account_name == name) return;
        s_account_name = name;
        // Clear stale data from previous account
        s_owned_skins.clear();
        s_queried_skins.clear();
        s_locally_marked.clear();
        s_has_data = false;
        s_generation++;
        s_status_message = "Click Refresh Owned to query skins";
    }

    void OwnedSkins::OnPong(void* eventArgs) {
        if (!eventArgs) return;
        auto* pong = (HoardPongPayload*)eventArgs;
        if (pong->api_version < 2) return;

        s_hs_available = true;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (pong->has_data) {
                s_status_message = "Hoard & Seek connected";
            } else {
                s_status_message = "Hoard & Seek has no data - refresh in H&S first";
            }
        }
    }

    void OwnedSkins::OnDataUpdated(void* eventArgs) {
        // H&S finished a data refresh — signal that we should re-query skins
        s_hs_available = true;
        s_data_updated = true;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Hoard & Seek data updated";
        }
    }

    bool OwnedSkins::ConsumeDataUpdatedFlag() {
        return s_data_updated.exchange(false);
    }

    void OwnedSkins::RequestOwnedSkinsViaApi() {
        if (!s_API || !s_hs_available) return;
        if (s_querying) return;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_querying = true;
            s_status_message = "Querying /v2/account/skins...";
        }

        std::string acctName;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            acctName = s_account_name;
        }
        HoardQueryApiRequest req{};
        req.api_version = HOARD_API_VERSION;
        strncpy(req.requester, SKINVENTORY_REQUESTER, sizeof(req.requester));
        strncpy(req.endpoint, "/v2/account/skins", sizeof(req.endpoint));
        strncpy(req.response_event, EV_SKINVENTORY_API_RESPONSE, sizeof(req.response_event));
        if (!acctName.empty())
            strncpy(req.account_name, acctName.c_str(), sizeof(req.account_name) - 1);

        s_API->Events_Raise(EV_HOARD_QUERY_API, &req);
    }

    void OwnedSkins::OnApiResponse(void* eventArgs) {
        if (!eventArgs) return;
        auto* resp = (HoardQueryApiResponse*)eventArgs;

        if (resp->status == HOARD_STATUS_DENIED) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Permission denied by Hoard & Seek";
            s_querying = false;
            return;
        }

        if (resp->status == HOARD_STATUS_PENDING) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Waiting for Hoard & Seek permission approval...";
            s_querying = false;
            return;
        }

        if (resp->status == HOARD_STATUS_OK) {
            // Check if truncated — if so, data is incomplete
            // Also check json_length vs buffer: /v2/account/skins can exceed 64KB
            if (resp->truncated || resp->json_length >= sizeof(resp->json)) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status_message = "API response truncated, falling back to batch query...";
                s_querying = false;
                // Caller should fall back to RequestOwnedSkins()
                return;
            }

            // Parse the JSON array of owned skin IDs
            try {
                json j = json::parse(resp->json);
                if (j.is_array()) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_owned_skins.clear();
                    for (const auto& id : j) {
                        s_owned_skins.insert(id.get<uint32_t>());
                    }
                    // Re-insert locally marked skins (from alerts) that stale API data may not include
                    for (uint32_t id : s_locally_marked) {
                        s_owned_skins.insert(id);
                    }
                    s_has_data = true;
                    s_querying = false;
                    s_generation++;
                    s_status_message = "Owned: " + std::to_string(s_owned_skins.size());
                }
            } catch (...) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status_message = "Failed to parse /v2/account/skins response";
                s_querying = false;
            }

            // Save to disk cache
            SaveToCache();
        }
    }

    void OwnedSkins::RequestOwnedSkins(const std::vector<uint32_t>& skin_ids) {
        if (!s_API || !s_hs_available || skin_ids.empty()) return;
        if (s_querying) return;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_pending_ids = skin_ids;
            s_batch_index = 0;
            s_querying = true;
            s_status_message = "Querying owned skins (batch)...";
        }

        SendNextBatch();
    }

    void OwnedSkins::SendNextBatch() {
        if (!s_API) return;

        // Prepare request under lock, then release lock BEFORE calling Events_Raise.
        // Nexus may dispatch events synchronously (inline), which would call our
        // OnSkinsResponse handler. If we held s_mutex here, that would deadlock.
        HoardQuerySkinsRequest req{};  // Stack-allocated (matches FlipOut pattern)
        bool allDone = false;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_batch_index >= s_pending_ids.size()) {
                s_querying = false;
                s_has_data = true;
                s_status_message = "Owned: " + std::to_string(s_owned_skins.size()) +
                                   " / " + std::to_string(s_queried_skins.size());
                allDone = true;
            }
        }
        if (allDone) {
            SaveToCache();
            return;
        }
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            size_t end = std::min(s_batch_index + 200, s_pending_ids.size());

            req.api_version = HOARD_API_VERSION;
            strncpy(req.requester, SKINVENTORY_REQUESTER, sizeof(req.requester));
            strncpy(req.response_event, EV_SKINVENTORY_SKINS_RESPONSE, sizeof(req.response_event));
            if (!s_account_name.empty())
                strncpy(req.account_name, s_account_name.c_str(), sizeof(req.account_name) - 1);

            req.id_count = 0;
            for (size_t i = s_batch_index; i < end; i++) {
                req.ids[req.id_count++] = s_pending_ids[i];
            }

            s_status_message = "Querying skins " + std::to_string(s_batch_index) +
                               "-" + std::to_string(end) + "/" + std::to_string(s_pending_ids.size());
        }
        // Lock released — safe to raise event now
        s_API->Events_Raise(EV_HOARD_QUERY_SKINS, &req);
    }

    void OwnedSkins::OnSkinsResponse(void* eventArgs) {
        if (!eventArgs) return;
        auto* resp = (HoardQuerySkinsResponse*)eventArgs;

        if (resp->status == HOARD_STATUS_DENIED) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Permission denied by Hoard & Seek";
            s_querying = false;
            return;
        }

        if (resp->status == HOARD_STATUS_PENDING) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Waiting for H&S permission approval...";
            s_querying = false;
            return;
        }

        if (resp->status == HOARD_STATUS_OK) {
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (uint32_t i = 0; i < resp->entry_count; i++) {
                    uint32_t id = resp->entries[i].id;
                    s_queried_skins.insert(id);
                    if (resp->entries[i].unlocked) {
                        s_owned_skins.insert(id);
                    }
                }
                s_batch_index += resp->entry_count;
            }

            // Defer next batch to Tick() to avoid deep recursion
            // (H&S delivers events synchronously inside Events_Raise)
            s_batch_pending = true;
        }
    }

    bool OwnedSkins::IsOwned(uint32_t skin_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_owned_skins.count(skin_id) > 0;
    }

    void OwnedSkins::MarkOwned(uint32_t skin_id) {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_owned_skins.insert(skin_id);
            s_locally_marked.insert(skin_id);
        }
        s_generation++;
        SaveToCache();
    }

    bool OwnedSkins::HasData() {
        return s_has_data;
    }

    bool OwnedSkins::IsHoardAndSeekAvailable() {
        return s_hs_available;
    }

    bool OwnedSkins::IsQuerying() {
        return s_querying;
    }

    std::string OwnedSkins::GetStatusMessage() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_status_message;
    }

    size_t OwnedSkins::GetOwnedCount() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_owned_skins.size();
    }

    size_t OwnedSkins::GetQueriedCount() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_queried_skins.size();
    }

    uint64_t OwnedSkins::GetGeneration() {
        return s_generation;
    }

    // --- Disk Cache ---

    bool OwnedSkins::SaveToCache() {
        if (s_data_dir.empty()) return false;
        try { std::filesystem::create_directories(s_data_dir); } catch (...) { return false; }

        std::string path = s_data_dir + "/owned_skins.json";
        json j = json::array();
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (uint32_t id : s_owned_skins) {
                j.push_back(id);
            }
        }

        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump();
        return true;
    }

    bool OwnedSkins::LoadFromCache() {
        if (s_data_dir.empty()) return false;
        std::string path = s_data_dir + "/owned_skins.json";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j;
            if (!j.is_array() || j.empty()) return false;

            std::lock_guard<std::mutex> lock(s_mutex);
            s_owned_skins.clear();
            for (const auto& id : j) {
                s_owned_skins.insert(id.get<uint32_t>());
            }
            return true;
        } catch (...) {
            return false;
        }
    }

}
