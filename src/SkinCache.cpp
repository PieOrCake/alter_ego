#include "SkinCache.h"
#include "HttpClient.h"

#include <nlohmann/json.hpp>
#include <thread>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <sstream>

using json = nlohmann::json;

namespace Skinventory {

    std::string SkinCache::s_data_dir;
    std::atomic<CacheStatus> SkinCache::s_status{CacheStatus::Empty};
    std::string SkinCache::s_status_message;
    std::mutex SkinCache::s_mutex;
    std::unordered_map<uint32_t, SkinInfo> SkinCache::s_skins;
    std::unordered_map<SkinCache::CategoryKey, std::vector<uint32_t>, SkinCache::CategoryKeyHash> SkinCache::s_category_index;

    void SkinCache::SetDataDirectory(const std::string& dir) {
        s_data_dir = dir;
    }

    bool SkinCache::EnsureDataDirectory() {
        if (s_data_dir.empty()) return false;
        try {
            std::filesystem::create_directories(s_data_dir);
            return true;
        } catch (...) {
            return false;
        }
    }

    CacheStatus SkinCache::GetStatus() {
        return s_status.load();
    }

    std::string SkinCache::GetStatusMessage() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_status_message;
    }

    std::optional<SkinInfo> SkinCache::GetSkin(uint32_t id) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_skins.find(id);
        if (it != s_skins.end()) return it->second;
        return std::nullopt;
    }

    std::vector<uint32_t> SkinCache::GetSkinsByCategory(const std::string& type,
                                                         const std::string& subtype,
                                                         const std::string& weight_class) {
        std::lock_guard<std::mutex> lock(s_mutex);
        CategoryKey key{type, subtype, weight_class};
        auto it = s_category_index.find(key);
        if (it != s_category_index.end()) return it->second;
        return {};
    }

    std::vector<std::string> SkinCache::GetArmorWeights() {
        return {"Heavy", "Medium", "Light", "Clothing"};
    }

    std::vector<std::string> SkinCache::GetArmorSlots(const std::string& weight_class) {
        (void)weight_class;
        return {"Helm", "Shoulders", "Coat", "Gloves", "Leggings", "Boots"};
    }

    std::vector<std::string> SkinCache::GetWeaponTypes() {
        return {
            "Axe", "Dagger", "Mace", "Pistol", "Scepter", "Sword",
            "Focus", "Shield", "Torch", "Warhorn",
            "Greatsword", "Hammer", "Longbow", "Rifle", "Short Bow", "Staff",
            "Harpoon Gun", "Spear", "Trident"
        };
    }

    size_t SkinCache::GetTotalSkinCount() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_skins.size();
    }

    size_t SkinCache::GetCategorySkinCount(const std::string& type,
                                           const std::string& subtype,
                                           const std::string& weight_class) {
        std::lock_guard<std::mutex> lock(s_mutex);
        CategoryKey key{type, subtype, weight_class};
        auto it = s_category_index.find(key);
        if (it != s_category_index.end()) return it->second.size();
        return 0;
    }

    void SkinCache::RebuildIndex() {
        // Called with lock held
        s_category_index.clear();
        for (const auto& [id, skin] : s_skins) {
            if (skin.type == "Armor") {
                CategoryKey key{skin.type, skin.subtype, skin.weight_class};
                s_category_index[key].push_back(id);
            } else if (skin.type == "Weapon") {
                CategoryKey key{skin.type, skin.subtype, ""};
                s_category_index[key].push_back(id);
            } else if (skin.type == "Back") {
                CategoryKey key{"Back", "", ""};
                s_category_index[key].push_back(id);
            }
        }
        // Sort each category by name
        for (auto& [key, ids] : s_category_index) {
            std::sort(ids.begin(), ids.end(), [&](uint32_t a, uint32_t b) {
                auto itA = s_skins.find(a);
                auto itB = s_skins.find(b);
                if (itA == s_skins.end() || itB == s_skins.end()) return a < b;
                return itA->second.name < itB->second.name;
            });
        }
    }

    bool SkinCache::SaveToDisk() {
        if (!EnsureDataDirectory()) return false;
        std::string path = s_data_dir + "/skin_cache.json";

        json j = json::array();
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (const auto& [id, skin] : s_skins) {
                json entry;
                entry["id"] = skin.id;
                entry["name"] = skin.name;
                entry["type"] = skin.type;
                entry["subtype"] = skin.subtype;
                entry["weight_class"] = skin.weight_class;
                entry["rarity"] = skin.rarity;
                entry["icon"] = skin.icon_url;
                j.push_back(entry);
            }
        }

        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump();
        file.close();
        return true;
    }

    bool SkinCache::LoadFromDisk() {
        std::string path = s_data_dir + "/skin_cache.json";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j;
            if (!j.is_array() || j.empty()) return false;

            std::lock_guard<std::mutex> lock(s_mutex);
            s_skins.clear();
            for (const auto& entry : j) {
                SkinInfo skin;
                skin.id = entry.value("id", (uint32_t)0);
                skin.name = entry.value("name", "");
                skin.type = entry.value("type", "");
                skin.subtype = entry.value("subtype", "");
                skin.weight_class = entry.value("weight_class", "");
                skin.rarity = entry.value("rarity", "");
                skin.icon_url = entry.value("icon", "");
                if (skin.id > 0) {
                    s_skins[skin.id] = skin;
                }
            }
            RebuildIndex();
            s_status = CacheStatus::Ready;
            s_status_message = "Loaded " + std::to_string(s_skins.size()) + " skins from cache";
            return true;
        } catch (...) {
            return false;
        }
    }

    void SkinCache::FetchAllSkinsAsync() {
        CacheStatus expected = s_status.load();
        if (expected == CacheStatus::Loading) return;
        s_status = CacheStatus::Loading;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Starting skin fetch...";
        }
        std::thread(FetchWorker).detach();
    }

    void SkinCache::UpdateCacheAsync() {
        CacheStatus expected = s_status.load();
        if (expected == CacheStatus::Loading || expected == CacheStatus::Updating) return;
        s_status = CacheStatus::Updating;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status_message = "Checking for new skins...";
        }
        std::thread(UpdateWorker).detach();
    }

    // Helper: fetch skin IDs from the API. Returns empty vector on failure.
    static std::vector<uint32_t> FetchSkinIdList() {
        std::string response = HttpClient::Get("https://api.guildwars2.com/v2/skins");
        if (response.empty()) return {};
        try {
            json j = json::parse(response);
            if (!j.is_array()) return {};
            std::vector<uint32_t> ids;
            ids.reserve(j.size());
            for (const auto& id : j) ids.push_back(id.get<uint32_t>());
            return ids;
        } catch (...) { return {}; }
    }

    // Helper: fetch skin details for a batch of IDs, append to 'out'.
    static void FetchSkinBatch(const std::vector<uint32_t>& ids, size_t start, size_t end,
                                std::unordered_map<uint32_t, SkinInfo>& out) {
        std::string ids_param;
        for (size_t j = start; j < end; j++) {
            if (!ids_param.empty()) ids_param += ",";
            ids_param += std::to_string(ids[j]);
        }
        std::string url = "https://api.guildwars2.com/v2/skins?ids=" + ids_param;
        std::string response = HttpClient::Get(url);
        if (response.empty()) return;
        try {
            json batch = json::parse(response);
            if (!batch.is_array()) return;
            for (const auto& entry : batch) {
                SkinInfo skin;
                skin.id = entry.value("id", (uint32_t)0);
                skin.name = entry.value("name", "");
                skin.type = entry.value("type", "");
                skin.rarity = entry.value("rarity", "");
                skin.icon_url = entry.value("icon", "");
                if (entry.contains("details") && entry["details"].is_object()) {
                    const auto& details = entry["details"];
                    skin.subtype = details.value("type", "");
                    skin.weight_class = details.value("weight_class", "");
                }
                if (entry.contains("flags") && entry["flags"].is_array()) {
                    for (const auto& f : entry["flags"]) skin.flags.push_back(f.get<std::string>());
                }
                if (entry.contains("restrictions") && entry["restrictions"].is_array()) {
                    for (const auto& r : entry["restrictions"]) skin.restrictions.push_back(r.get<std::string>());
                }
                if (skin.id > 0) out[skin.id] = std::move(skin);
            }
        } catch (...) {}
    }

    void SkinCache::FetchWorker() {
        try {
            // Step 1: Get all skin IDs
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status_message = "Fetching skin ID list...";
            }
            auto ids = FetchSkinIdList();
            if (ids.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status = CacheStatus::Error;
                s_status_message = "Failed to fetch skin IDs";
                return;
            }

            // Step 2: Fetch skin details in batches of 200
            const size_t BATCH_SIZE = 200;
            size_t total = ids.size();
            std::unordered_map<uint32_t, SkinInfo> fetched;

            for (size_t i = 0; i < total; i += BATCH_SIZE) {
                size_t end = std::min(i + BATCH_SIZE, total);
                {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_status_message = "Fetching skins " + std::to_string(i) + "/" + std::to_string(total) + "...";
                }
                FetchSkinBatch(ids, i, end, fetched);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            if (fetched.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status = CacheStatus::Error;
                s_status_message = "No skin data received";
                return;
            }

            // Step 3: Store and index
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_skins = std::move(fetched);
                RebuildIndex();
                s_status_message = "Loaded " + std::to_string(s_skins.size()) + " skins";
            }
            s_status = CacheStatus::Ready;

            // Step 4: Save to disk
            SaveToDisk();

        } catch (...) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status = CacheStatus::Error;
            s_status_message = "Unexpected error during skin fetch";
        }
    }

    void SkinCache::UpdateWorker() {
        try {
            // Step 1: Get current API skin IDs
            auto api_ids = FetchSkinIdList();
            if (api_ids.empty()) {
                // Network error — stay Ready with existing cache, just log
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status = CacheStatus::Ready;
                s_status_message = "Update check failed (network), using cached data";
                return;
            }

            // Step 2: Find IDs not in our cache
            std::vector<uint32_t> new_ids;
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (uint32_t id : api_ids) {
                    if (s_skins.find(id) == s_skins.end()) {
                        new_ids.push_back(id);
                    }
                }
            }

            if (new_ids.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status = CacheStatus::Ready;
                s_status_message = "Cache up to date (" + std::to_string(s_skins.size()) + " skins)";
                return;
            }

            // Step 3: Fetch only the new skins
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status_message = "Fetching " + std::to_string(new_ids.size()) + " new skins...";
            }

            const size_t BATCH_SIZE = 200;
            std::unordered_map<uint32_t, SkinInfo> fetched;
            for (size_t i = 0; i < new_ids.size(); i += BATCH_SIZE) {
                size_t end = std::min(i + BATCH_SIZE, new_ids.size());
                FetchSkinBatch(new_ids, i, end, fetched);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            // Step 4: Merge into existing cache
            if (!fetched.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (auto& [id, skin] : fetched) {
                    s_skins[id] = std::move(skin);
                }
                RebuildIndex();
                s_status_message = "Updated: " + std::to_string(s_skins.size()) + " skins (" +
                                   std::to_string(fetched.size()) + " new)";
            } else {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_status_message = "Cache up to date (" + std::to_string(s_skins.size()) + " skins)";
            }

            s_status = CacheStatus::Ready;
            SaveToDisk();

        } catch (...) {
            // On error during update, stay Ready with existing data
            std::lock_guard<std::mutex> lock(s_mutex);
            s_status = CacheStatus::Ready;
            s_status_message = "Update error, using cached data";
        }
    }

}
