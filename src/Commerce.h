#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>

namespace Skinventory {

    struct PriceInfo {
        uint32_t item_id = 0;
        int buy_price = 0;   // Highest buy order in copper
        int sell_price = 0;  // Lowest sell listing in copper
        int buy_quantity = 0;
        int sell_quantity = 0;
        bool tradeable = false;
        std::chrono::steady_clock::time_point fetched_at;
    };

    // Format copper value as gold/silver/copper string
    std::string FormatCoins(int copper);

    class Commerce {
    public:
        // Set data directory for item map cache
        static void SetDataDirectory(const std::string& dir);

        // Fetch TP price for a single skin (async, on-demand)
        static void FetchPriceForSkin(uint32_t skin_id);

        // Batch fetch prices for multiple skins (for shopping list)
        static void FetchPricesForSkins(const std::vector<uint32_t>& skin_ids);

        // Get cached price for a skin. Returns nullptr if not fetched yet.
        static const PriceInfo* GetPrice(uint32_t skin_id);

        // Get vendor price for a skin (0 if not a vendor skin). In copper.
        static int GetVendorPrice(uint32_t skin_id);

        // Is a price fetch in progress?
        static bool IsFetching();
        static std::string GetFetchStatus();

        // Item map: skin_id -> item_id (for TP lookups)
        // Must be built once from /v2/items
        static void BuildItemMapAsync();
        static bool IsItemMapReady();
        static uint32_t GetItemIdForSkin(uint32_t skin_id);

        // Persistence for item map
        static bool LoadItemMap();
        static bool SaveItemMap();

    private:
        static void FetchPriceWorker(std::vector<uint32_t> item_ids,
                                      std::unordered_map<uint32_t, uint32_t> item_to_skin);
        static void ItemMapWorker();

        static std::string s_data_dir;
        static std::mutex s_mutex;

        // skin_id -> PriceInfo
        static std::unordered_map<uint32_t, PriceInfo> s_prices;

        // skin_id -> item_id (cheapest tradeable item for each skin)
        static std::unordered_map<uint32_t, uint32_t> s_skin_to_item;

        // skin_id -> vendor price in copper (cultural armor)
        static std::unordered_map<uint32_t, int> s_vendor_prices;

        static std::atomic<bool> s_fetching;
        static std::string s_fetch_status;
        static std::atomic<bool> s_item_map_ready;
        static std::atomic<bool> s_item_map_building;
    };

}
