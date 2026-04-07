#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace AlterEgo {

    // Chat link type bytes
    enum ChatLinkType : uint8_t {
        LINK_COIN          = 0x01,
        LINK_ITEM          = 0x02,
        LINK_NPC_TEXT      = 0x03,
        LINK_MAP           = 0x04,
        LINK_PVP_GAME      = 0x05,
        LINK_SKILL         = 0x06,
        LINK_TRAIT         = 0x07,
        LINK_USER          = 0x08,
        LINK_RECIPE        = 0x09,
        LINK_SKIN          = 0x0A,
        LINK_OUTFIT        = 0x0B,
        LINK_WVW_OBJECTIVE = 0x0C,
        LINK_BUILD         = 0x0D,
        LINK_ACHIEVEMENT   = 0x0E,
        LINK_WARDROBE_TMPL = 0x0F,
    };

    // Item link flags (byte after item ID)
    enum ItemLinkFlags : uint8_t {
        ITEM_FLAG_SKINNED    = 0x80,  // 1 << 7
        ITEM_FLAG_UPGRADE1   = 0x40,  // 1 << 6
        ITEM_FLAG_UPGRADE2   = 0x20,  // 1 << 5
        ITEM_FLAG_NAME_KEY   = 0x10,  // 1 << 4
        ITEM_FLAG_DESC_KEY   = 0x08,  // 1 << 3
    };

    // Profession codes for build templates
    enum ProfessionCode : uint8_t {
        PROF_GUARDIAN    = 1,
        PROF_WARRIOR     = 2,
        PROF_ENGINEER    = 3,
        PROF_RANGER      = 4,
        PROF_THIEF       = 5,
        PROF_ELEMENTALIST = 6,
        PROF_MESMER      = 7,
        PROF_NECROMANCER = 8,
        PROF_REVENANT    = 9,
    };

    // Weapon type IDs used in build template chat links
    enum WeaponTypeId : uint16_t {
        WEAPON_AXE       = 5,
        WEAPON_LONGBOW   = 35,
        WEAPON_DAGGER    = 47,
        WEAPON_FOCUS     = 49,
        WEAPON_GREATSWORD = 50,
        WEAPON_HAMMER    = 51,
        WEAPON_MACE      = 53,
        WEAPON_PISTOL    = 54,
        WEAPON_RIFLE     = 85,
        WEAPON_SCEPTER   = 86,
        WEAPON_SHIELD    = 87,
        WEAPON_STAFF     = 89,
        WEAPON_SWORD     = 90,
        WEAPON_TORCH     = 102,
        WEAPON_WARHORN   = 103,
        WEAPON_SHORTBOW  = 107,
        WEAPON_SPEAR     = 265,
    };

    // Decoded item link
    struct DecodedItemLink {
        uint32_t item_id = 0;
        int count = 1;
        uint32_t skin_id = 0;       // 0 = no skin override
        uint32_t upgrade1_id = 0;   // 0 = no upgrade
        uint32_t upgrade2_id = 0;   // 0 = no upgrade
    };

    // Decoded build template link
    struct DecodedBuildLink {
        uint8_t profession = 0;
        struct SpecChoice {
            uint8_t spec_id = 0;
            uint8_t traits[3] = {0, 0, 0}; // 0=none, 1=top, 2=mid, 3=bottom
        } specs[3];
        uint16_t terrestrial_skills[5] = {0}; // heal, util1, util2, util3, elite (palette IDs)
        uint16_t aquatic_skills[5] = {0};
        // Ranger pets
        uint8_t pets[4] = {0};      // terr1, terr2, aqua1, aqua2
        // Revenant legends
        uint8_t legends[4] = {0};   // terr1, terr2, aqua1, aqua2
        uint16_t inactive_rev_skills[6] = {0}; // inactive legend utilities
        // Weapons (SotO+)
        std::vector<uint16_t> weapons;
        // Skill overrides (Weaponmaster)
        std::vector<uint32_t> skill_overrides;
    };

    // Decoded wardrobe template link
    struct DecodedWardrobeLink {
        uint16_t aquabreather_skin = 0;
        uint16_t backpack_skin = 0;
        uint16_t backpack_dyes[4] = {0};
        uint16_t chest_skin = 0;
        uint16_t chest_dyes[4] = {0};
        uint16_t boots_skin = 0;
        uint16_t boots_dyes[4] = {0};
        uint16_t gloves_skin = 0;
        uint16_t gloves_dyes[4] = {0};
        uint16_t helm_skin = 0;
        uint16_t helm_dyes[4] = {0};
        uint16_t leggings_skin = 0;
        uint16_t leggings_dyes[4] = {0};
        uint16_t shoulders_skin = 0;
        uint16_t shoulders_dyes[4] = {0};
        uint16_t outfit_id = 0;
        uint16_t outfit_dyes[4] = {0};
        uint16_t aquatic_weapon_a = 0;
        uint16_t aquatic_weapon_b = 0;
        uint16_t weapon_a_main = 0;
        uint16_t weapon_a_off = 0;
        uint16_t weapon_b_main = 0;
        uint16_t weapon_b_off = 0;
        uint16_t visibility_flags = 0;
    };

    class ChatLink {
    public:
        // --- Encoding ---

        // Generate item chat link: [&Base64...]
        static std::string EncodeItem(uint32_t item_id, int count = 1,
                                       uint32_t skin_id = 0,
                                       uint32_t upgrade1 = 0,
                                       uint32_t upgrade2 = 0);

        // Generate skill chat link
        static std::string EncodeSkill(uint32_t skill_id);

        // Generate trait chat link
        static std::string EncodeTrait(uint32_t trait_id);

        // Generate skin chat link
        static std::string EncodeSkin(uint32_t skin_id);

        // Generate build template chat link
        static std::string EncodeBuild(const DecodedBuildLink& build);

        // --- Decoding ---

        // Detect link type from a chat link string
        static ChatLinkType DetectType(const std::string& link);

        // Decode an item chat link
        static bool DecodeItem(const std::string& link, DecodedItemLink& out);

        // Decode a build template chat link
        static bool DecodeBuild(const std::string& link, DecodedBuildLink& out);

        // Decode a wardrobe template chat link
        static bool DecodeWardrobe(const std::string& link, DecodedWardrobeLink& out);

        // --- Utilities ---

        // Base64 encode/decode
        static std::string Base64Encode(const uint8_t* data, size_t len);
        static std::vector<uint8_t> Base64Decode(const std::string& encoded);

        // Extract the base64 payload from [&...] wrapper
        static bool ExtractPayload(const std::string& link, std::string& payload);

        // Get profession name from code
        static const char* ProfessionName(uint8_t code);

        // Get weapon name from type ID
        static const char* WeaponName(uint16_t type_id);
    };

}
