#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <optional>

namespace Skinventory {

    // Skin metadata from /v2/skins
    struct SkinInfo {
        uint32_t id = 0;
        std::string name;
        std::string type;        // "Armor", "Weapon", "Back", "Gathering"
        std::string subtype;     // Armor: "Helm","Shoulders","Coat","Gloves","Leggings","Boots"
                                 // Weapon: "Sword","Axe","Dagger", etc.
        std::string weight_class; // Armor only: "Heavy","Medium","Light","Clothing"
        std::string rarity;
        std::string icon_url;
        std::vector<std::string> flags;
        std::vector<std::string> restrictions;
    };

    enum class CacheStatus {
        Empty,
        Loading,
        Ready,
        Updating, // Ready + background update in progress
        Error
    };

    class SkinCache {
    public:
        // Set the data directory for cache files
        static void SetDataDirectory(const std::string& dir);

        // Begin async fetch of all skin data from /v2/skins (first-time only)
        static void FetchAllSkinsAsync();

        // Incremental update: fetch only new skins since last cache
        static void UpdateCacheAsync();

        // Current status
        static CacheStatus GetStatus();
        static std::string GetStatusMessage();

        // Query cached skins (returns copy — safe across threads)
        static std::optional<SkinInfo> GetSkin(uint32_t id);
        static std::vector<uint32_t> GetSkinsByCategory(const std::string& type,
                                                         const std::string& subtype,
                                                         const std::string& weight_class);

        // Get all armor weight classes
        static std::vector<std::string> GetArmorWeights();
        // Get all armor subtypes (slots) for a weight class
        static std::vector<std::string> GetArmorSlots(const std::string& weight_class);
        // Get all weapon subtypes
        static std::vector<std::string> GetWeaponTypes();

        // Total skin counts
        static size_t GetTotalSkinCount();
        static size_t GetCategorySkinCount(const std::string& type,
                                           const std::string& subtype,
                                           const std::string& weight_class);

        // Persistence
        static bool LoadFromDisk();
        static bool SaveToDisk();

    private:
        static void FetchWorker();
        static void UpdateWorker();
        static bool EnsureDataDirectory();

        static std::string s_data_dir;
        static std::atomic<CacheStatus> s_status;
        static std::string s_status_message;
        static std::mutex s_mutex;

        // All skins indexed by ID
        static std::unordered_map<uint32_t, SkinInfo> s_skins;

        // Category indices: type -> subtype -> weight_class -> [skin_ids]
        // For weapons: type="Weapon", subtype="Sword", weight_class=""
        // For armor: type="Armor", subtype="Helm", weight_class="Heavy"
        struct CategoryKey {
            std::string type;
            std::string subtype;
            std::string weight_class;
            bool operator==(const CategoryKey& o) const {
                return type == o.type && subtype == o.subtype && weight_class == o.weight_class;
            }
        };
        struct CategoryKeyHash {
            size_t operator()(const CategoryKey& k) const {
                size_t h = std::hash<std::string>{}(k.type);
                h ^= std::hash<std::string>{}(k.subtype) << 1;
                h ^= std::hash<std::string>{}(k.weight_class) << 2;
                return h;
            }
        };
        static std::unordered_map<CategoryKey, std::vector<uint32_t>, CategoryKeyHash> s_category_index;

        static void RebuildIndex();
    };

}
