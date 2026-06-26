#pragma once
#include "FontResolve.h"
#include "nexus/Nexus.h"
struct ImFont;

namespace AlterEgo { namespace FontManager {
    void    Initialize(AddonAPI_t* api);
    // Ready TTF -> ImFont*; Nexus-default face or not-yet-loaded -> nullptr.
    ImFont* ResolveDefault(const Font::Config& cfg);
    void    Shutdown();
}}
