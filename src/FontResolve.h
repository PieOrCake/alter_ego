#pragma once
// Pure font-resolution layer. NO ImGui / Nexus dependencies — header-only and
// host-unit-testable. Computes the stable Nexus font identifier for a config.
#include <string>
#include <cstdint>
#include <cmath>
#include <cstdio>

namespace AlterEgo { namespace Font {

    enum class Face : int { NexusDefault = 0, Bundled = 1, Custom = 2 };

    struct Config {
        Face        face = Face::NexusDefault;
        std::string customPath;   // full path to a .ttf when face == Custom
        float       px   = 16.0f; // requested pixel size for TTF faces
    };

    inline bool IsDefault(const Config& c) { return c.face == Face::NexusDefault; }

    // 32-bit FNV-1a hash of a string (stable, for custom-path identifiers).
    inline uint32_t Fnv32(const std::string& s) {
        uint32_t h = 2166136261u;
        for (unsigned char ch : s) { h ^= ch; h *= 16777619u; }
        return h;
    }

    // Stable Nexus identifier. Empty for the Nexus-default face (no TTF). px is
    // rounded to the nearest int so 18.0 and 17.99 share one id (one baked atlas entry).
    inline std::string Identifier(const Config& c) {
        if (c.face == Face::NexusDefault) return std::string();
        int px = (int)std::lround(c.px);
        if (c.face == Face::Bundled)
            return "AlterEgo_F_BUNDLED_" + std::to_string(px);
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%08x", Fnv32(c.customPath));
        return std::string("AlterEgo_F_C") + buf + "_" + std::to_string(px);
    }

}} // namespace AlterEgo::Font
