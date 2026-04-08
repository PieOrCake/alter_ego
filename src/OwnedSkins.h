#pragma once

#include <vector>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <string>
#include "nexus/Nexus.h"

#include <nlohmann/json.hpp>

namespace Skinventory {

    class OwnedSkins {
    public:
        // Initialize: subscribe to H&S events
        static void Initialize(AddonAPI_t* api);
        // Cleanup: unsubscribe from events
        static void Shutdown();

        // Call once per frame to send deferred batch queries
        static void Tick();

        // Set data directory for disk cache
        static void SetDataDirectory(const std::string& dir);

        // Ping H&S to check availability
        static void PingHoardAndSeek();

        // Request owned skins via H&S API proxy (/v2/account/skins) — single request
        static void RequestOwnedSkinsViaApi();

        // Request owned skin data from H&S (batched, up to 200 at a time)
        // Fallback if API proxy response is truncated.
        static void RequestOwnedSkins(const std::vector<uint32_t>& skin_ids);

        // Check if a specific skin is owned (returns false if unknown/not yet queried)
        static bool IsOwned(uint32_t skin_id);

        // Has H&S responded with data?
        static bool HasData();

        // Is H&S available?
        static bool IsHoardAndSeekAvailable();

        // Is a query in progress?
        static bool IsQuerying();

        // Was H&S data updated since last check? (auto-clears on read)
        static bool ConsumeDataUpdatedFlag();

        // Status message for UI
        static std::string GetStatusMessage();

        // Get counts
        static size_t GetOwnedCount();
        static size_t GetQueriedCount();

    private:
        static void OnPong(void* eventArgs);
        static void OnSkinsResponse(void* eventArgs);
        static void OnApiResponse(void* eventArgs);
        static void OnDataUpdated(void* eventArgs);

        static bool SaveToCache();
        static bool LoadFromCache();

        static AddonAPI_t* s_API;
        static std::string s_data_dir;
        static std::unordered_set<uint32_t> s_owned_skins;
        static std::unordered_set<uint32_t> s_queried_skins; // All skins we've asked about
        static std::mutex s_mutex;
        static std::atomic<bool> s_has_data;
        static std::atomic<bool> s_hs_available;
        static std::atomic<bool> s_querying;
        static std::atomic<bool> s_data_updated;
        static std::string s_status_message;

        // Batch query state
        static std::vector<uint32_t> s_pending_ids;
        static size_t s_batch_index;
        static std::atomic<bool> s_batch_pending; // deferred to next Tick()
        static void SendNextBatch();
    };

}
