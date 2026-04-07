#include "ChatLink.h"
#include <cstring>
#include <algorithm>

namespace AlterEgo {

// --- Base64 ---

static const char B64_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string ChatLink::Base64Encode(const uint8_t* data, size_t len) {
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        unsigned int n = ((unsigned int)data[i]) << 16;
        if (i + 1 < len) n |= ((unsigned int)data[i + 1]) << 8;
        if (i + 2 < len) n |= (unsigned int)data[i + 2];
        out += B64_TABLE[(n >> 18) & 0x3F];
        out += B64_TABLE[(n >> 12) & 0x3F];
        out += (i + 1 < len) ? B64_TABLE[(n >> 6) & 0x3F] : '=';
        out += (i + 2 < len) ? B64_TABLE[n & 0x3F] : '=';
    }
    return out;
}

static int B64_DECODE_CHAR(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::vector<uint8_t> ChatLink::Base64Decode(const std::string& encoded) {
    std::vector<uint8_t> out;
    if (encoded.empty()) return out;
    out.reserve((encoded.size() / 4) * 3);

    for (size_t i = 0; i < encoded.size(); i += 4) {
        int a = (i < encoded.size()) ? B64_DECODE_CHAR(encoded[i]) : 0;
        int b = (i + 1 < encoded.size()) ? B64_DECODE_CHAR(encoded[i + 1]) : 0;
        int c = (i + 2 < encoded.size()) ? B64_DECODE_CHAR(encoded[i + 2]) : 0;
        int d = (i + 3 < encoded.size()) ? B64_DECODE_CHAR(encoded[i + 3]) : 0;

        if (a < 0) a = 0;
        if (b < 0) b = 0;
        if (c < 0) c = 0;
        if (d < 0) d = 0;

        unsigned int triple = (a << 18) | (b << 12) | (c << 6) | d;
        out.push_back((triple >> 16) & 0xFF);
        if (i + 2 < encoded.size() && encoded[i + 2] != '=')
            out.push_back((triple >> 8) & 0xFF);
        if (i + 3 < encoded.size() && encoded[i + 3] != '=')
            out.push_back(triple & 0xFF);
    }
    return out;
}

// --- Payload extraction ---

bool ChatLink::ExtractPayload(const std::string& link, std::string& payload) {
    // Expected format: [&Base64Data]
    size_t start = link.find("[&");
    if (start == std::string::npos) return false;
    size_t end = link.find(']', start);
    if (end == std::string::npos) return false;
    payload = link.substr(start + 2, end - start - 2);
    return !payload.empty();
}

// --- Type detection ---

ChatLinkType ChatLink::DetectType(const std::string& link) {
    std::string payload;
    if (!ExtractPayload(link, payload)) return (ChatLinkType)0;
    auto bytes = Base64Decode(payload);
    if (bytes.empty()) return (ChatLinkType)0;
    return (ChatLinkType)bytes[0];
}

// --- Item encoding ---

std::string ChatLink::EncodeItem(uint32_t item_id, int count,
                                  uint32_t skin_id, uint32_t upgrade1, uint32_t upgrade2) {
    std::vector<uint8_t> buf;
    buf.push_back(LINK_ITEM);
    buf.push_back((uint8_t)std::min(count, 250));

    // Item ID (3 bytes LE) + padding byte
    buf.push_back((uint8_t)(item_id & 0xFF));
    buf.push_back((uint8_t)((item_id >> 8) & 0xFF));
    buf.push_back((uint8_t)((item_id >> 16) & 0xFF));

    // Flags byte
    uint8_t flags = 0;
    if (skin_id) flags |= ITEM_FLAG_SKINNED;
    if (upgrade1) flags |= ITEM_FLAG_UPGRADE1;
    if (upgrade2) flags |= ITEM_FLAG_UPGRADE2;
    buf.push_back(flags);

    // Optional fields (only present if flag is set)
    if (skin_id) {
        buf.push_back((uint8_t)(skin_id & 0xFF));
        buf.push_back((uint8_t)((skin_id >> 8) & 0xFF));
        buf.push_back((uint8_t)((skin_id >> 16) & 0xFF));
        buf.push_back(0);
    }
    if (upgrade1) {
        buf.push_back((uint8_t)(upgrade1 & 0xFF));
        buf.push_back((uint8_t)((upgrade1 >> 8) & 0xFF));
        buf.push_back((uint8_t)((upgrade1 >> 16) & 0xFF));
        buf.push_back(0);
    }
    if (upgrade2) {
        buf.push_back((uint8_t)(upgrade2 & 0xFF));
        buf.push_back((uint8_t)((upgrade2 >> 8) & 0xFF));
        buf.push_back((uint8_t)((upgrade2 >> 16) & 0xFF));
        buf.push_back(0);
    }

    return "[&" + Base64Encode(buf.data(), buf.size()) + "]";
}

// --- Simple link encoders ---

static std::string EncodeSimpleLink(uint8_t type, uint32_t id) {
    uint8_t buf[5] = {0};
    buf[0] = type;
    buf[1] = (uint8_t)(id & 0xFF);
    buf[2] = (uint8_t)((id >> 8) & 0xFF);
    buf[3] = (uint8_t)((id >> 16) & 0xFF);
    buf[4] = 0;
    return "[&" + ChatLink::Base64Encode(buf, 5) + "]";
}

std::string ChatLink::EncodeSkill(uint32_t skill_id) {
    return EncodeSimpleLink(LINK_SKILL, skill_id);
}

std::string ChatLink::EncodeTrait(uint32_t trait_id) {
    return EncodeSimpleLink(LINK_TRAIT, trait_id);
}

std::string ChatLink::EncodeSkin(uint32_t skin_id) {
    return EncodeSimpleLink(LINK_SKIN, skin_id);
}

// --- Build template encoding ---

static void WriteU16LE(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back((uint8_t)(val & 0xFF));
    buf.push_back((uint8_t)((val >> 8) & 0xFF));
}

static void WriteU32LE(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back((uint8_t)(val & 0xFF));
    buf.push_back((uint8_t)((val >> 8) & 0xFF));
    buf.push_back((uint8_t)((val >> 16) & 0xFF));
    buf.push_back((uint8_t)((val >> 24) & 0xFF));
}

std::string ChatLink::EncodeBuild(const DecodedBuildLink& build) {
    std::vector<uint8_t> buf;
    buf.push_back(LINK_BUILD);
    buf.push_back(build.profession);

    // 3 specialization lines (2 bytes each)
    for (int i = 0; i < 3; i++) {
        buf.push_back(build.specs[i].spec_id);
        // Traits encoded as 2-bit values in reverse order, first 2 bits unused
        uint8_t trait_byte = 0;
        trait_byte |= (build.specs[i].traits[0] & 0x03);        // bits 0-1: first choice
        trait_byte |= (build.specs[i].traits[1] & 0x03) << 2;   // bits 2-3: second choice
        trait_byte |= (build.specs[i].traits[2] & 0x03) << 4;   // bits 4-5: third choice
        buf.push_back(trait_byte);
    }

    // 10 skill slots (2 bytes each, palette IDs LE)
    // Order: terr_heal, aqua_heal, terr_util1, aqua_util1, terr_util2, aqua_util2,
    //        terr_util3, aqua_util3, terr_elite, aqua_elite
    WriteU16LE(buf, build.terrestrial_skills[0]); // Heal
    WriteU16LE(buf, build.aquatic_skills[0]);
    WriteU16LE(buf, build.terrestrial_skills[1]); // Util 1
    WriteU16LE(buf, build.aquatic_skills[1]);
    WriteU16LE(buf, build.terrestrial_skills[2]); // Util 2
    WriteU16LE(buf, build.aquatic_skills[2]);
    WriteU16LE(buf, build.terrestrial_skills[3]); // Util 3
    WriteU16LE(buf, build.aquatic_skills[3]);
    WriteU16LE(buf, build.terrestrial_skills[4]); // Elite
    WriteU16LE(buf, build.aquatic_skills[4]);

    // Profession-specific: 16 bytes
    if (build.profession == PROF_RANGER) {
        // 4 pet bytes + 12 unused
        buf.push_back(build.pets[0]);
        buf.push_back(build.pets[1]);
        buf.push_back(build.pets[2]);
        buf.push_back(build.pets[3]);
        for (int i = 0; i < 12; i++) buf.push_back(0);
    } else if (build.profession == PROF_REVENANT) {
        // 4 legend bytes
        buf.push_back(build.legends[0]);
        buf.push_back(build.legends[1]);
        buf.push_back(build.legends[2]);
        buf.push_back(build.legends[3]);
        // 6 inactive legend skill palette IDs (2 bytes each)
        for (int i = 0; i < 6; i++) {
            WriteU16LE(buf, build.inactive_rev_skills[i]);
        }
        // 0 padding to reach 16 bytes total for prof-specific section isn't needed
        // since we wrote 4 + 12 = 16 bytes
    } else {
        // 16 zero bytes
        for (int i = 0; i < 16; i++) buf.push_back(0);
    }

    // Weapons array (SotO+)
    buf.push_back((uint8_t)build.weapons.size());
    for (auto w : build.weapons) {
        WriteU16LE(buf, w);
    }

    // Skill overrides (Weaponmaster)
    buf.push_back((uint8_t)build.skill_overrides.size());
    for (auto s : build.skill_overrides) {
        WriteU32LE(buf, s);
    }

    return "[&" + Base64Encode(buf.data(), buf.size()) + "]";
}

// --- Item decoding ---

static uint32_t ReadU24LE(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

static uint16_t ReadU16LE(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

bool ChatLink::DecodeItem(const std::string& link, DecodedItemLink& out) {
    std::string payload;
    if (!ExtractPayload(link, payload)) return false;
    auto bytes = Base64Decode(payload);
    if (bytes.size() < 6 || bytes[0] != LINK_ITEM) return false;

    out.count = bytes[1];
    out.item_id = ReadU24LE(&bytes[2]);

    uint8_t flags = bytes[5];
    size_t offset = 6;

    if (flags & ITEM_FLAG_SKINNED) {
        if (offset + 4 > bytes.size()) return false;
        out.skin_id = ReadU24LE(&bytes[offset]);
        offset += 4;
    }
    if (flags & ITEM_FLAG_UPGRADE1) {
        if (offset + 4 > bytes.size()) return false;
        out.upgrade1_id = ReadU24LE(&bytes[offset]);
        offset += 4;
    }
    if (flags & ITEM_FLAG_UPGRADE2) {
        if (offset + 4 > bytes.size()) return false;
        out.upgrade2_id = ReadU24LE(&bytes[offset]);
        offset += 4;
    }

    return true;
}

// --- Build template decoding ---

bool ChatLink::DecodeBuild(const std::string& link, DecodedBuildLink& out) {
    std::string payload;
    if (!ExtractPayload(link, payload)) return false;
    auto bytes = Base64Decode(payload);
    if (bytes.size() < 2 || bytes[0] != LINK_BUILD) return false;

    memset(&out, 0, sizeof(out));
    out.profession = bytes[1];

    // 3 specialization lines starting at byte 2
    size_t offset = 2;
    for (int i = 0; i < 3; i++) {
        if (offset + 2 > bytes.size()) return true; // partial decode OK
        out.specs[i].spec_id = bytes[offset];
        uint8_t tb = bytes[offset + 1];
        out.specs[i].traits[0] = tb & 0x03;
        out.specs[i].traits[1] = (tb >> 2) & 0x03;
        out.specs[i].traits[2] = (tb >> 4) & 0x03;
        offset += 2;
    }

    // 10 skill palette IDs (2 bytes each LE)
    if (offset + 20 > bytes.size()) return true;
    out.terrestrial_skills[0] = ReadU16LE(&bytes[offset]);      offset += 2; // heal
    out.aquatic_skills[0]     = ReadU16LE(&bytes[offset]);      offset += 2;
    out.terrestrial_skills[1] = ReadU16LE(&bytes[offset]);      offset += 2; // util1
    out.aquatic_skills[1]     = ReadU16LE(&bytes[offset]);      offset += 2;
    out.terrestrial_skills[2] = ReadU16LE(&bytes[offset]);      offset += 2; // util2
    out.aquatic_skills[2]     = ReadU16LE(&bytes[offset]);      offset += 2;
    out.terrestrial_skills[3] = ReadU16LE(&bytes[offset]);      offset += 2; // util3
    out.aquatic_skills[3]     = ReadU16LE(&bytes[offset]);      offset += 2;
    out.terrestrial_skills[4] = ReadU16LE(&bytes[offset]);      offset += 2; // elite
    out.aquatic_skills[4]     = ReadU16LE(&bytes[offset]);      offset += 2;

    // Profession-specific: 16 bytes
    if (out.profession == PROF_RANGER) {
        if (offset + 4 <= bytes.size()) {
            out.pets[0] = bytes[offset];
            out.pets[1] = bytes[offset + 1];
            out.pets[2] = bytes[offset + 2];
            out.pets[3] = bytes[offset + 3];
        }
        offset += 16;
    } else if (out.profession == PROF_REVENANT) {
        if (offset + 4 <= bytes.size()) {
            out.legends[0] = bytes[offset];
            out.legends[1] = bytes[offset + 1];
            out.legends[2] = bytes[offset + 2];
            out.legends[3] = bytes[offset + 3];
        }
        offset += 4;
        // 6 inactive legend skills
        for (int i = 0; i < 6; i++) {
            if (offset + 2 <= bytes.size()) {
                out.inactive_rev_skills[i] = ReadU16LE(&bytes[offset]);
            }
            offset += 2;
        }
    } else {
        offset += 16;
    }

    // Weapons array (SotO+)
    if (offset < bytes.size()) {
        uint8_t weapon_count = bytes[offset++];
        for (int i = 0; i < weapon_count && offset + 2 <= bytes.size(); i++) {
            out.weapons.push_back(ReadU16LE(&bytes[offset]));
            offset += 2;
        }
    }

    // Skill overrides (Weaponmaster)
    if (offset < bytes.size()) {
        uint8_t override_count = bytes[offset++];
        for (int i = 0; i < override_count && offset + 4 <= bytes.size(); i++) {
            out.skill_overrides.push_back(ReadU32LE(&bytes[offset]));
            offset += 4;
        }
    }

    return true;
}

// --- Wardrobe template decoding ---

bool ChatLink::DecodeWardrobe(const std::string& link, DecodedWardrobeLink& out) {
    std::string payload;
    if (!ExtractPayload(link, payload)) return false;
    auto bytes = Base64Decode(payload);
    if (bytes.size() < 2 || bytes[0] != LINK_WARDROBE_TMPL) return false;

    memset(&out, 0, sizeof(out));

    // All values are uint16_t LE, sequentially
    size_t offset = 1;
    auto read16 = [&]() -> uint16_t {
        if (offset + 2 > bytes.size()) return 0;
        uint16_t val = ReadU16LE(&bytes[offset]);
        offset += 2;
        return val;
    };

    out.aquabreather_skin = read16();
    out.backpack_skin = read16();
    for (int i = 0; i < 4; i++) out.backpack_dyes[i] = read16();
    out.chest_skin = read16();
    for (int i = 0; i < 4; i++) out.chest_dyes[i] = read16();
    out.boots_skin = read16();
    for (int i = 0; i < 4; i++) out.boots_dyes[i] = read16();
    out.gloves_skin = read16();
    for (int i = 0; i < 4; i++) out.gloves_dyes[i] = read16();
    out.helm_skin = read16();
    for (int i = 0; i < 4; i++) out.helm_dyes[i] = read16();
    out.leggings_skin = read16();
    for (int i = 0; i < 4; i++) out.leggings_dyes[i] = read16();
    out.shoulders_skin = read16();
    for (int i = 0; i < 4; i++) out.shoulders_dyes[i] = read16();
    out.outfit_id = read16();
    for (int i = 0; i < 4; i++) out.outfit_dyes[i] = read16();
    out.aquatic_weapon_a = read16();
    out.aquatic_weapon_b = read16();
    out.weapon_a_main = read16();
    out.weapon_a_off = read16();
    out.weapon_b_main = read16();
    out.weapon_b_off = read16();
    out.visibility_flags = read16();

    return true;
}

// --- Utility lookups ---

const char* ChatLink::ProfessionName(uint8_t code) {
    switch (code) {
        case PROF_GUARDIAN:     return "Guardian";
        case PROF_WARRIOR:      return "Warrior";
        case PROF_ENGINEER:     return "Engineer";
        case PROF_RANGER:       return "Ranger";
        case PROF_THIEF:        return "Thief";
        case PROF_ELEMENTALIST: return "Elementalist";
        case PROF_MESMER:       return "Mesmer";
        case PROF_NECROMANCER:  return "Necromancer";
        case PROF_REVENANT:     return "Revenant";
        default:                return "Unknown";
    }
}

const char* ChatLink::WeaponName(uint16_t type_id) {
    switch (type_id) {
        case WEAPON_AXE:        return "Axe";
        case WEAPON_LONGBOW:    return "Longbow";
        case WEAPON_DAGGER:     return "Dagger";
        case WEAPON_FOCUS:      return "Focus";
        case WEAPON_GREATSWORD: return "Greatsword";
        case WEAPON_HAMMER:     return "Hammer";
        case WEAPON_MACE:       return "Mace";
        case WEAPON_PISTOL:     return "Pistol";
        case WEAPON_RIFLE:      return "Rifle";
        case WEAPON_SCEPTER:    return "Scepter";
        case WEAPON_SHIELD:     return "Shield";
        case WEAPON_STAFF:      return "Staff";
        case WEAPON_SWORD:      return "Sword";
        case WEAPON_TORCH:      return "Torch";
        case WEAPON_WARHORN:    return "Warhorn";
        case WEAPON_SHORTBOW:   return "Shortbow";
        case WEAPON_SPEAR:      return "Spear";
        default:                return "Unknown";
    }
}

} // namespace AlterEgo
