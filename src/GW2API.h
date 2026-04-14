#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>
#include "nexus/Nexus.h"

namespace AlterEgo {

    enum class FetchStatus {
        Idle,
        InProgress,
        Success,
        Error
    };

    // H&S connection state
    enum class HoardStatus {
        Unknown,        // Haven't pinged yet
        Unavailable,    // Ping sent, no pong received
        Available,      // Pong received, H&S is running
        PermPending,    // Permission popup shown to user
        PermDenied,     // User denied permission
        Ready           // Permission granted, data flowing
    };

    // Equipment slot on a character
    struct EquipmentItem {
        uint32_t id = 0;
        std::string slot;           // "Helm", "Shoulders", "Coat", etc.
        uint32_t skin = 0;          // Skin ID (transmuted appearance)
        std::vector<uint32_t> upgrades;    // Sigil/Rune IDs
        std::vector<uint32_t> infusions;
        std::vector<int> dyes;      // Dye color IDs (up to 4)
        std::string binding;        // "Character" or "Account"
        std::string bound_to;       // Character name if character-bound
        int tab = 0;                // Equipment tab number
        uint32_t stat_id = 0;       // Selected itemstat ID (for legendary/ascended)
        std::map<std::string, int> attributes; // e.g. {"Power": 251, "Precision": 179}
    };

    // Cached itemstat info from /v2/itemstats (public, no auth)
    struct ItemStatInfo {
        uint32_t id = 0;
        std::string name;           // e.g. "Berserker's", "Minstrel's"
        std::vector<std::string> attributes; // e.g. {"Power", "Precision", "CritDamage"}
    };

    // A gear slot in a saved build (user-entered stats/upgrades)
    struct BuildGearSlot {
        std::string slot;           // "Helm", "Shoulders", "Coat", etc.
        uint32_t stat_id = 0;       // /v2/itemstats ID
        std::string stat_name;      // "Berserker's", etc.
        std::string rune;           // Rune name (armor only)
        uint32_t rune_id = 0;       // Rune item ID (for compact encoding)
        std::string sigil;          // Sigil name (weapon only)
        uint32_t sigil_id = 0;      // Sigil item ID (for compact encoding)
        std::string infusion;       // Infusion name
        std::string weapon_type;    // "Axe", "Sword", etc. (weapon slots only)
    };

    // Weapon skills for a profession+weapon combination
    struct ProfessionWeaponData {
        bool mainhand = false;
        bool offhand = false;
        bool two_handed = false;
        uint32_t specialization = 0; // 0 = core, else elite spec ID
        uint32_t skills[5] = {0};    // Weapon_1..Weapon_5 skill IDs (default/no qualifier)
        // Thief dual wield: offhand weapon type -> Weapon_3 skill ID
        std::map<std::string, uint32_t> dual_wield;
        // Elementalist attunements: attunement name -> 5 skill IDs
        std::map<std::string, std::array<uint32_t, 5>> attunement_skills;
    };

    // A build template specialization line
    struct SpecLine {
        uint32_t spec_id = 0;       // /v2/specializations ID
        int traits[3] = {0, 0, 0};  // Selected major trait indices (0=none, 1=top, 2=mid, 3=bottom)
    };

    // A build template's skill loadout
    struct SkillBar {
        uint32_t heal = 0;
        uint32_t utilities[3] = {0, 0, 0};
        uint32_t elite = 0;
    };

    // Ranger pets
    struct RangerPets {
        uint32_t terrestrial[2] = {0, 0};
        uint32_t aquatic[2] = {0, 0};
    };

    // Revenant legends
    struct RevenantLegends {
        std::string terrestrial[2];
        std::string aquatic[2];
    };

    // A single build template
    struct BuildTemplate {
        int tab = 0;
        bool is_active = false;
        std::string name;
        std::string profession;     // Profession name from API
        SpecLine specializations[3];
        SkillBar terrestrial_skills;
        SkillBar aquatic_skills;
        RangerPets pets;
        RevenantLegends legends;
        std::vector<uint32_t> weapons; // Weapon type IDs
    };

    // Cached info about an item from /v2/items (public, no auth)
    struct ItemInfo {
        uint32_t id = 0;
        std::string name;
        std::string icon_url;
        std::string rarity;
        std::string type;
        std::string description;
        std::string chat_link;
        nlohmann::json details;     // Full details block for stats, etc.
    };

    // Cached info about a skin from /v2/skins (public, no auth)
    struct SkinInfo {
        uint32_t id = 0;
        std::string name;
        std::string icon_url;
        std::string type;
        std::string rarity;
        int dye_slot_count = -1; // Number of dye channels (-1 = unknown)
        std::vector<std::string> dye_slot_materials; // Per-channel material: "cloth", "leather", "metal"
    };

    // Cached specialization info from /v2/specializations (public, no auth)
    struct SpecializationInfo {
        uint32_t id = 0;
        std::string name;
        std::string icon_url;
        std::string background_url;
        std::string profession;
        std::string profession_icon_url;       // Profession/elite emblem (small)
        std::string profession_icon_big_url;   // Profession/elite emblem (large)
        bool elite = false;
        std::vector<uint32_t> minor_traits;
        std::vector<uint32_t> major_traits;
    };

    // Cached trait info from /v2/traits (public, no auth)
    struct TraitInfo {
        uint32_t id = 0;
        std::string name;
        std::string icon_url;
        std::string description;
        int tier = 0;               // 1=Adept, 2=Master, 3=Grandmaster
        int order = 0;              // 0=top, 1=mid, 2=bottom
        std::string slot;           // "Major" or "Minor"
        nlohmann::json facts;       // Tooltip facts array
    };

    // Cached skill info from /v2/skills (public, no auth)
    struct SkillInfo {
        uint32_t id = 0;
        std::string name;
        std::string icon_url;
        std::string description;
        std::string type;           // "Heal", "Utility", "Elite", "Weapon", etc.
        nlohmann::json facts;       // Tooltip facts array
    };

    // Cached profession info from /v2/professions (public, no auth)
    struct ProfessionInfo {
        std::string name;
        std::string icon_url;
        std::string icon_big_url;
    };

    // Dye color info from /v2/colors (public, no auth)
    struct DyeColor {
        int id = 0;
        std::string name;
        int r = 0, g = 0, b = 0;           // Base RGB (fallback)
        int cloth_r = 0, cloth_g = 0, cloth_b = 0;
        int leather_r = 0, leather_g = 0, leather_b = 0;
        int metal_r = 0, metal_g = 0, metal_b = 0;
    };

    // A character on the account
    struct Character {
        std::string name;
        std::string race;
        std::string gender;
        std::string profession;
        int level = 0;
        int age = 0;                // Seconds played
        std::string created;        // ISO date
        std::string last_modified;  // ISO date — last login/modification
        int deaths = 0;
        std::vector<std::string> crafting;      // Crafting disciplines (active first)
        std::vector<int> crafting_levels;
        std::vector<bool> crafting_active;      // Whether each discipline is active
        std::vector<EquipmentItem> equipment;
        std::vector<BuildTemplate> build_tabs;
        std::map<int, std::string> equipment_tab_names; // tab number -> name
        int active_build_tab = 0;
        int active_equipment_tab = 0;
        std::vector<std::string> heropoints;    // Completed hero challenge IDs (e.g. "0-29")
        std::string account_name;              // Which H&S account owns this character
    };

    // Item location result from H&S cache query
    struct ItemLocationEntry {
        std::string location;       // e.g. "Bank", character name
        std::string sublocation;    // e.g. "Bag", "Equipped"
        int count = 0;
        std::string account_name;   // Which account this location belongs to
    };

    // Account info from H&S v3
    struct AccountInfo {
        std::string name;           // GW2 account name (e.g. "PieOrCake.7635")
        std::string label;          // User-assigned friendly name (e.g. "Main")
        std::string display_name;   // label if non-empty, else name
        int64_t last_updated = 0;
        bool validated = false;
        std::vector<std::string> characters; // Character names on this account
    };

    struct ItemLocationResult {
        uint32_t item_id = 0;
        std::string name;
        int total_count = 0;
        std::vector<ItemLocationEntry> locations;
    };

    // Game mode tags for saved builds
    enum class GameMode {
        PvE,
        WvW,
        PvP,
        Raid,
        Fractal,
        Other
    };

    // A saved build in the build library
    struct SavedBuild {
        std::string id;             // Unique ID (timestamp-based)
        std::string name;           // User-given name
        std::string chat_link;      // Original chat link [&...]
        std::string profession;     // Profession name (decoded)
        GameMode game_mode = GameMode::PvE;
        std::string notes;          // User notes
        time_t created = 0;
        // Decoded build data (populated on import)
        SpecLine specializations[3];
        SkillBar terrestrial_skills; // Skill IDs (resolved from palette)
        SkillBar aquatic_skills;
        RangerPets pets;
        uint8_t legend_codes[4] = {0}; // Revenant legend codes
        std::vector<uint32_t> weapons;
        // Gear (user-entered)
        std::map<std::string, BuildGearSlot> gear; // slot name -> gear data
        std::string rune_name;      // Shared rune for all armor pieces
        uint32_t rune_id = 0;       // Shared rune item ID
        std::string relic_name;     // Relic name
        uint32_t relic_id = 0;      // Relic item ID
    };

    // Response event names for H&S queries
    #define EV_AE_CHAR_LIST_RESP   "EV_ALTER_EGO_CHAR_LIST"
    #define EV_AE_CHAR_DATA_RESP   "EV_ALTER_EGO_CHAR_DATA"
    #define EV_AE_SKIN_UNLOCK_RESP "EV_ALTER_EGO_SKIN_UNLOCKS"
    #define EV_AE_ITEM_LOC_RESP    "EV_ALTER_EGO_ITEM_LOC"
    #define EV_AE_ACCOUNTS_RESP    "EV_ALTER_EGO_ACCOUNTS"

    class GW2API {
    public:
        // --- Nexus integration ---
        static void Initialize(AddonAPI_t* api);
        static void Shutdown();

        // Data path helper
        static std::string GetDataDirectory();

        // --- H&S integration ---
        static HoardStatus GetHoardStatus();
        static void PingHoard();                    // Check if H&S is available
        static void RequestCharacterRefresh(const std::string& priorityChar = ""); // Fetch all characters via H&S
        static void RequestCharacterList();              // Fetch just the character name list
        static void RequestCharacterRefreshSelected(const std::vector<std::string>& names); // Fetch specific characters

        // H&S cached queries (synchronous, from H&S in-memory cache)
        static void QuerySkinUnlocks(const std::vector<uint32_t>& skin_ids);
        static void QueryItemLocation(uint32_t item_id);

        // H&S event handlers (registered via Events_Subscribe)
        static void OnHoardPong(void* eventArgs);
        static void OnHoardDataUpdated(void* eventArgs);
        static void OnCharListResponse(void* eventArgs);
        static void OnCharDataResponse(void* eventArgs);
        static void OnSkinUnlocksResponse(void* eventArgs);
        static void OnItemLocationResponse(void* eventArgs);
        static void OnAccountsResponse(void* eventArgs);
        static void OnAccountsChanged(void* eventArgs);

        // --- Multi-account ---
        static void QueryAccounts();                     // Fetch account list from H&S
        static uint32_t GetAccountCount();               // Number of configured accounts (0 = unknown)
        static const std::vector<AccountInfo>& GetAccounts();
        static const std::string& GetCurrentAccountName(); // Account for the logged-in character
        static void SetCurrentAccountFromCharacter(const std::string& charName); // Map character to account
        static bool IsMultiAccount();                    // account_count > 1
        static bool HasAccountsData();                   // true if accounts have been queried
        static std::vector<std::string> GetAllCharacterNamesFromAccounts(const std::string& accountFilter = "");
        static std::string GetAccountForCharacter(const std::string& charName);

        // --- Fetch state ---
        static FetchStatus GetFetchStatus();
        static const std::string& GetFetchStatusMessage();
        static std::string GetCurrentFetchCharName();

        // --- Query ---
        static bool HasCharacterData();
        static time_t GetLastUpdated();

        // Character access
        static const std::vector<Character>& GetCharacters();
        static const Character* GetCharacter(const std::string& name);
        static const std::vector<std::string>& GetPendingCharNames();

        // Skin unlock status (from H&S cache)
        static bool IsSkinUnlocked(uint32_t skin_id);  // true if unlocked, false if unknown/locked
        static bool HasSkinUnlockData(uint32_t skin_id); // true if we've queried this skin
        static void MarkSkinUnlocked(uint32_t skin_id);  // mark a skin as unlocked (e.g. from Events: Alerts)

        // Item location (from H&S cache)
        static const ItemLocationResult* GetItemLocation(uint32_t item_id);

        // Item/Skin/Spec/Trait/Skill/Color caches (public endpoints, fetched directly)
        static const ItemInfo* GetItemInfo(uint32_t item_id);
        static uint32_t FindItemIdByName(const std::string& name); // searches item cache + name cache
        static void CacheItemNameId(const std::string& name, uint32_t id); // add to persistent name→ID cache
        static bool LoadItemNameCache();
        static bool SaveItemNameCache();
        static const SkinInfo* GetSkinInfo(uint32_t skin_id);
        static const SpecializationInfo* GetSpecInfo(uint32_t spec_id);
        static const TraitInfo* GetTraitInfo(uint32_t trait_id);
        static const SkillInfo* GetSkillInfo(uint32_t skill_id);
        static const DyeColor* GetDyeColor(int color_id);
        static const ItemStatInfo* GetItemStatInfo(uint32_t stat_id);
        static const std::unordered_map<uint32_t, ItemStatInfo>& GetAllItemStats();
        static void FetchAllItemStatsAsync();
        static const ProfessionInfo* GetProfessionInfo(const std::string& profession);
        static void FetchProfessionInfoAsync(const std::string& profession);
        static const std::map<std::string, ProfessionWeaponData>* GetProfessionWeapons(const std::string& profession);

        // Fetch detail data on-demand (direct HTTP — no auth needed)
        static void FetchItemDetails(const std::vector<uint32_t>& item_ids); // synchronous
        static void FetchItemDetailsAsync(const std::vector<uint32_t>& item_ids);
        static void FetchSkinDetailsAsync(const std::vector<uint32_t>& skin_ids);
        static void FetchSpecDetailsAsync(const std::vector<uint32_t>& spec_ids);
        static void FetchTraitDetailsAsync(const std::vector<uint32_t>& trait_ids);
        static void FetchSkillDetailsAsync(const std::vector<uint32_t>& skill_ids);
        static void FetchItemStatDetailsAsync(const std::vector<uint32_t>& stat_ids);
        static void FetchDyeColorsAsync();

        // Persistence
        static bool LoadCharacterData();
        static bool SaveCharacterData();

        // --- Build Library ---
        static const std::vector<SavedBuild>& GetSavedBuilds();
        static bool AddSavedBuild(SavedBuild build);
        static bool RemoveSavedBuild(const std::string& id);
        static bool UpdateSavedBuild(const std::string& id, const std::string& name, const std::string& notes);
        static bool ReorderSavedBuild(int fromIdx, int toIdx);
        static bool LoadBuildLibrary();
        static bool SaveBuildLibrary();

        // --- Palette ID mapping ---
        static uint32_t GetSkillIdFromPalette(const std::string& profession, uint16_t palette_id);
        static uint16_t GetPaletteIdFromSkill(const std::string& profession, uint32_t skill_id);
        static bool HasPaletteData(const std::string& profession);
        static void FetchProfessionPaletteAsync(const std::string& profession);

    private:
        static AddonAPI_t* s_api;
        static HoardStatus s_hoard_status;
        static FetchStatus s_fetch_status;
        static std::string s_fetch_message;

        // Multi-account state
        static uint32_t s_account_count;                 // from pong/data_updated
        static std::vector<AccountInfo> s_accounts;      // from QueryAccounts
        static std::string s_current_account_name;       // resolved from MumbleLink char
        static std::unordered_map<std::string, std::string> s_char_to_account; // char name -> account_name
        static bool s_accounts_queried;                  // have we queried accounts at least once

        static std::vector<Character> s_characters;
        static bool s_has_character_data;
        static time_t s_last_updated;

        // Pending character fetch state
        static std::vector<std::string> s_pending_char_names;
        static int s_chars_fetched;
        static int s_chars_total;
        static std::string s_priority_char_name;
        static int s_pending_char_idx; // index into s_pending_char_names for next char to fetch
        static bool s_list_only_mode;  // true = just fetch name list, don't start char data fetch
        static int s_multi_acct_list_idx;  // index into s_accounts for multi-account char list fetch (-1 = not active)

        // Sub-endpoint fetch state
        // Phase 0: /core
        // Phase 1: /equipmenttabs (just IDs)
        // Phase 2: /equipmenttabs/:N (one per tab)
        // Phase 3: /buildtabs?tabs=all
        static Character s_current_char;
        static int s_fetch_phase;
        static std::vector<int> s_equip_tab_ids;
        static int s_equip_tab_idx;

        // H&S cached data
        static std::unordered_map<uint32_t, bool> s_skin_unlocks;  // skin_id -> unlocked
        static std::unordered_map<uint32_t, ItemLocationResult> s_item_locations;

        // Caches (public endpoint data)
        static std::unordered_map<uint32_t, ItemInfo> s_item_cache;
        static std::unordered_map<uint32_t, SkinInfo> s_skin_cache;
        static std::unordered_map<uint32_t, SpecializationInfo> s_spec_cache;
        static std::unordered_map<uint32_t, TraitInfo> s_trait_cache;
        static std::unordered_map<uint32_t, SkillInfo> s_skill_cache;
        static std::unordered_map<int, DyeColor> s_dye_cache;
        static std::unordered_map<uint32_t, ItemStatInfo> s_itemstat_cache;

        // Profession info cache
        static std::unordered_map<std::string, ProfessionInfo> s_profession_cache;
        static std::unordered_map<std::string, std::map<std::string, ProfessionWeaponData>> s_profession_weapons;

        // Build library
        static std::vector<SavedBuild> s_saved_builds;

        // Persistent name→ID cache (survives restarts, grows organically)
        static std::unordered_map<std::string, uint32_t> s_item_name_id_cache;
        static bool s_item_name_cache_dirty;

        // Palette ID mapping: profession -> (palette_id -> skill_id)
        static std::unordered_map<std::string, std::unordered_map<uint16_t, uint32_t>> s_palette_to_skill;
        static std::unordered_map<std::string, std::unordered_map<uint32_t, uint16_t>> s_skill_to_palette;
        static std::unordered_set<std::string> s_palette_fetching; // professions currently being fetched

        static std::mutex s_mutex;

        // HTTP helpers (for public/unauthenticated endpoints only)
        static std::string HttpGet(const std::string& url);
        static bool EnsureDataDirectory();

        // Batch fetch helpers (public endpoints, synchronous)
        static void FetchSkinDetails(const std::vector<uint32_t>& skin_ids);
        static void FetchTraitDetails(const std::vector<uint32_t>& trait_ids);

        // Parse helpers
        static Character ParseCharacterJson(const nlohmann::json& cj);
        static EquipmentItem ParseEquipmentItem(const nlohmann::json& item_json);
        static BuildTemplate ParseBuildTemplate(const nlohmann::json& tab_json);

        // Sub-endpoint fetch for current character (phase 0/1/2)
        static void FetchCharacterPhase();
    };

}
