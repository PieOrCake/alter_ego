#include "Commerce.h"
#include "HttpClient.h"
#include "SkinCache.h"

#include <nlohmann/json.hpp>
#include <thread>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

using json = nlohmann::json;

namespace Skinventory {

    std::string Commerce::s_data_dir;
    std::mutex Commerce::s_mutex;
    std::unordered_map<uint32_t, PriceInfo> Commerce::s_prices;
    std::unordered_map<uint32_t, uint32_t> Commerce::s_skin_to_item;
    std::unordered_map<uint32_t, int> Commerce::s_vendor_prices;
    std::atomic<bool> Commerce::s_fetching{false};
    std::string Commerce::s_fetch_status;
    std::atomic<bool> Commerce::s_item_map_ready{false};
    std::atomic<bool> Commerce::s_item_map_building{false};

    std::string FormatCoins(int copper) {
        if (copper <= 0) return "0c";

        int gold = copper / 10000;
        int silver = (copper % 10000) / 100;
        int cop = copper % 100;

        std::string result;
        if (gold > 0) result += std::to_string(gold) + "g ";
        if (silver > 0 || gold > 0) result += std::to_string(silver) + "s ";
        result += std::to_string(cop) + "c";
        return result;
    }

    void Commerce::SetDataDirectory(const std::string& dir) {
        s_data_dir = dir;
    }

    const PriceInfo* Commerce::GetPrice(uint32_t skin_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_prices.find(skin_id);
        if (it != s_prices.end()) return &it->second;
        return nullptr;
    }

    int Commerce::GetVendorPrice(uint32_t skin_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_vendor_prices.find(skin_id);
        if (it != s_vendor_prices.end()) return it->second;
        return 0;
    }

    // Cultural armor vendor prices (coin) by tier and slot.
    // Prices are fixed and never change.
    static int GetCulturalArmorPrice(int level, const std::string& slot) {
        // Tier from item level: T1=35, T2=60, T3=80
        int tier = 0;
        if (level <= 35) tier = 1;
        else if (level <= 60) tier = 2;
        else tier = 3;

        // slot from item details.type: Boots, Coat, Gloves, Helm, Leggings, Shoulders
        std::string s = slot;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);

        if (tier == 1) {
            if (s == "boots")     return 3400;
            if (s == "gloves")    return 3400;
            if (s == "helm")      return 4000;
            if (s == "shoulders") return 4000;
            if (s == "leggings")  return 6000;
            if (s == "coat")      return 8000;
        } else if (tier == 2) {
            if (s == "boots")     return 20000;
            if (s == "gloves")    return 20000;
            if (s == "helm")      return 20000;
            if (s == "shoulders") return 20000;
            if (s == "leggings")  return 35000;
            if (s == "coat")      return 50000;
        } else if (tier == 3) {
            if (s == "boots")     return 100000;
            if (s == "gloves")    return 100000;
            if (s == "helm")      return 100000;
            if (s == "shoulders") return 100000;
            if (s == "leggings")  return 175000;
            if (s == "coat")      return 250000;
        }
        return 0;
    }

    bool Commerce::IsFetching() {
        return s_fetching;
    }

    std::string Commerce::GetFetchStatus() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_fetch_status;
    }

    bool Commerce::IsItemMapReady() {
        return s_item_map_ready;
    }

    uint32_t Commerce::GetItemIdForSkin(uint32_t skin_id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_skin_to_item.find(skin_id);
        if (it != s_skin_to_item.end()) return it->second;
        return 0;
    }

    void Commerce::FetchPriceForSkin(uint32_t skin_id) {
        if (!s_item_map_ready) return;

        uint32_t item_id = GetItemIdForSkin(skin_id);
        if (item_id == 0) {
            // No tradeable item for this skin
            std::lock_guard<std::mutex> lock(s_mutex);
            PriceInfo info;
            info.item_id = 0;
            info.tradeable = false;
            info.fetched_at = std::chrono::steady_clock::now();
            s_prices[skin_id] = info;
            return;
        }

        // Check cache freshness (5 min)
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            auto it = s_prices.find(skin_id);
            if (it != s_prices.end()) {
                auto age = std::chrono::steady_clock::now() - it->second.fetched_at;
                if (age < std::chrono::minutes(5)) return; // Still fresh
            }
        }

        std::unordered_map<uint32_t, uint32_t> item_to_skin;
        item_to_skin[item_id] = skin_id;
        std::vector<uint32_t> items = {item_id};

        std::thread(FetchPriceWorker, items, item_to_skin).detach();
    }

    void Commerce::FetchPricesForSkins(const std::vector<uint32_t>& skin_ids) {
        if (!s_item_map_ready || s_fetching) return;

        std::vector<uint32_t> item_ids;
        std::unordered_map<uint32_t, uint32_t> item_to_skin;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (uint32_t skin_id : skin_ids) {
                auto it = s_skin_to_item.find(skin_id);
                if (it == s_skin_to_item.end() || it->second == 0) {
                    // Mark as non-tradeable
                    PriceInfo info;
                    info.tradeable = false;
                    info.fetched_at = std::chrono::steady_clock::now();
                    s_prices[skin_id] = info;
                    continue;
                }

                // Check cache freshness
                auto pit = s_prices.find(skin_id);
                if (pit != s_prices.end()) {
                    auto age = std::chrono::steady_clock::now() - pit->second.fetched_at;
                    if (age < std::chrono::minutes(5)) continue;
                }

                item_ids.push_back(it->second);
                item_to_skin[it->second] = skin_id;
            }
        }

        if (item_ids.empty()) return;

        s_fetching = true;
        std::thread(FetchPriceWorker, item_ids, item_to_skin).detach();
    }

    void Commerce::FetchPriceWorker(std::vector<uint32_t> item_ids,
                                     std::unordered_map<uint32_t, uint32_t> item_to_skin) {
        const size_t BATCH_SIZE = 200;
        size_t total = item_ids.size();

        for (size_t i = 0; i < total; i += BATCH_SIZE) {
            size_t end = std::min(i + BATCH_SIZE, total);
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_fetch_status = "Fetching prices " + std::to_string(i) + "/" + std::to_string(total);
            }

            std::string ids_param;
            for (size_t j = i; j < end; j++) {
                if (!ids_param.empty()) ids_param += ",";
                ids_param += std::to_string(item_ids[j]);
            }

            std::string url = "https://api.guildwars2.com/v2/commerce/prices?ids=" + ids_param;
            std::string response = HttpClient::Get(url);
            if (response.empty()) continue;

            try {
                json batch = json::parse(response);
                if (!batch.is_array()) continue;

                std::lock_guard<std::mutex> lock(s_mutex);
                for (const auto& entry : batch) {
                    uint32_t item_id = entry.value("id", (uint32_t)0);
                    auto skin_it = item_to_skin.find(item_id);
                    if (skin_it == item_to_skin.end()) continue;

                    PriceInfo info;
                    info.item_id = item_id;
                    info.tradeable = true;
                    info.fetched_at = std::chrono::steady_clock::now();

                    if (entry.contains("buys") && entry["buys"].is_object()) {
                        info.buy_price = entry["buys"].value("unit_price", 0);
                        info.buy_quantity = entry["buys"].value("quantity", 0);
                    }
                    if (entry.contains("sells") && entry["sells"].is_object()) {
                        info.sell_price = entry["sells"].value("unit_price", 0);
                        info.sell_quantity = entry["sells"].value("quantity", 0);
                    }

                    s_prices[skin_it->second] = info;
                }
            } catch (...) {
                continue;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_fetch_status = "";
        }
        s_fetching = false;
    }

    // --- Item Map: skin_id -> item_id ---

    bool Commerce::SaveItemMap() {
        if (s_data_dir.empty()) return false;
        try { std::filesystem::create_directories(s_data_dir); } catch (...) { return false; }

        std::string path = s_data_dir + "/item_map.json";
        json j = json::object();
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (const auto& [skin_id, item_id] : s_skin_to_item) {
                j[std::to_string(skin_id)] = item_id;
            }
        }

        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump();

        // Save vendor prices
        std::string vpPath = s_data_dir + "/vendor_prices.json";
        json vj = json::object();
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (const auto& [skin_id, price] : s_vendor_prices) {
                vj[std::to_string(skin_id)] = price;
            }
        }
        std::ofstream vpFile(vpPath);
        if (vpFile.is_open()) vpFile << vj.dump();

        return true;
    }

    bool Commerce::LoadItemMap() {
        std::string path = s_data_dir + "/item_map.json";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j;
            if (!j.is_object() || j.empty()) return false;

            std::lock_guard<std::mutex> lock(s_mutex);
            s_skin_to_item.clear();
            for (auto it = j.begin(); it != j.end(); ++it) {
                uint32_t skin_id = std::stoul(it.key());
                uint32_t item_id = it.value().get<uint32_t>();
                s_skin_to_item[skin_id] = item_id;
            }
            s_item_map_ready = true;
        } catch (...) {
            return false;
        }

        // Load vendor prices
        std::string vpPath = s_data_dir + "/vendor_prices.json";
        std::ifstream vpFile(vpPath);
        if (vpFile.is_open()) {
            try {
                json vj;
                vpFile >> vj;
                std::lock_guard<std::mutex> lock(s_mutex);
                for (auto it = vj.begin(); it != vj.end(); ++it) {
                    uint32_t skin_id = std::stoul(it.key());
                    int price = it.value().get<int>();
                    s_vendor_prices[skin_id] = price;
                }
            } catch (...) {}
        }

        // If vendor prices are empty, the item map was built before vendor detection.
        // Force a rebuild.
        if (s_vendor_prices.empty()) {
            s_item_map_ready = false;
            return false;
        }

        return true;
    }

    void Commerce::BuildItemMapAsync() {
        if (s_item_map_building || s_item_map_ready) return;
        s_item_map_building = true;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_fetch_status = "Building item map...";
        }
        std::thread(ItemMapWorker).detach();
    }

    void Commerce::ItemMapWorker() {
        try {
            // Step 1: Get all item IDs from the GW2 API
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_fetch_status = "Building item map (one-time, cached after)...";
            }
            std::string response = HttpClient::Get("https://api.guildwars2.com/v2/items");
            if (response.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_fetch_status = "Item map failed: could not reach GW2 API";
                s_item_map_building = false;
                return;
            }

            json all_ids;
            try {
                all_ids = json::parse(response);
            } catch (...) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_fetch_status = "Item map failed: invalid response";
                s_item_map_building = false;
                return;
            }

            if (!all_ids.is_array()) {
                s_item_map_building = false;
                return;
            }

            std::vector<uint32_t> ids;
            ids.reserve(all_ids.size());
            for (const auto& id : all_ids) {
                ids.push_back(id.get<uint32_t>());
            }

            // Step 2: Fetch items in batches, extract default_skin for Armor/Weapon types
            const size_t BATCH_SIZE = 200;
            size_t total = ids.size();
            std::unordered_map<uint32_t, uint32_t> skin_map;
            std::unordered_map<uint32_t, std::string> skin_map_rarity; // rarity of stored item
            std::unordered_map<uint32_t, int> vendor_map;

            for (size_t i = 0; i < total; i += BATCH_SIZE) {
                size_t end = std::min(i + BATCH_SIZE, total);
                int pct = (int)((i * 100) / total);
                {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_fetch_status = "Building item map " + std::to_string(pct) + "% ("
                                   + std::to_string(skin_map.size()) + " skin mappings found)";
                }

                std::string ids_param;
                for (size_t j = i; j < end; j++) {
                    if (!ids_param.empty()) ids_param += ",";
                    ids_param += std::to_string(ids[j]);
                }

                std::string url = "https://api.guildwars2.com/v2/items?ids=" + ids_param;
                std::string batch_response = HttpClient::Get(url);
                if (batch_response.empty()) continue;

                try {
                    json batch = json::parse(batch_response);
                    if (!batch.is_array()) continue;

                    for (const auto& item : batch) {
                        std::string type = item.value("type", "");
                        if (type != "Armor" && type != "Weapon") continue;

                        uint32_t default_skin = item.value("default_skin", (uint32_t)0);
                        if (default_skin == 0) continue;

                        uint32_t item_id = item.value("id", (uint32_t)0);
                        if (item_id == 0) continue;

                        // Prefer item whose rarity matches the skin's rarity
                        std::string itemRarity = item.value("rarity", "");
                        auto existing = skin_map.find(default_skin);
                        if (existing == skin_map.end()) {
                            skin_map[default_skin] = item_id;
                            skin_map_rarity[default_skin] = itemRarity;
                        } else {
                            // Replace only if new item is a better rarity match
                            auto skinOpt = SkinCache::GetSkin(default_skin);
                            if (skinOpt && !skinOpt->rarity.empty()) {
                                const std::string& oldRarity = skin_map_rarity[default_skin];
                                bool oldMatches = (oldRarity == skinOpt->rarity);
                                bool newMatches = (itemRarity == skinOpt->rarity);
                                if (newMatches && !oldMatches) {
                                    skin_map[default_skin] = item_id;
                                    skin_map_rarity[default_skin] = itemRarity;
                                }
                            }
                        }

                        // Detect cultural armor: Armor with race restrictions
                        if (type == "Armor" && item.contains("restrictions") &&
                            item["restrictions"].is_array() && !item["restrictions"].empty()) {
                            bool hasRace = false;
                            for (const auto& r : item["restrictions"]) {
                                std::string rs = r.get<std::string>();
                                if (rs == "Asura" || rs == "Charr" || rs == "Human" ||
                                    rs == "Norn" || rs == "Sylvari") {
                                    hasRace = true;
                                    break;
                                }
                            }
                            if (hasRace) {
                                int level = item.value("level", 0);
                                std::string slot;
                                if (item.contains("details") && item["details"].is_object()) {
                                    slot = item["details"].value("type", "");
                                }
                                int vendorPrice = GetCulturalArmorPrice(level, slot);
                                if (vendorPrice > 0) {
                                    vendor_map[default_skin] = vendorPrice;
                                }
                            }
                        }
                    }
                } catch (...) {
                    continue;
                }

                // Minimal delay to avoid API rate limits
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // Step 3: Store
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_skin_to_item = std::move(skin_map);
                s_vendor_prices = std::move(vendor_map);
                s_fetch_status = "Item map ready (" + std::to_string(s_skin_to_item.size()) + " skin-to-item mappings, "
                               + std::to_string(s_vendor_prices.size()) + " vendor skins)";
            }
            s_item_map_ready = true;
            s_item_map_building = false;

            // Save to disk for future loads
            SaveItemMap();

        } catch (...) {
            s_item_map_building = false;
        }
    }

}
