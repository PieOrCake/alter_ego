#pragma once
// Declarations shared across multiple addon modules.
// Implementations live in dllmain.cpp.

#include <string>
#include "nexus/Nexus.h"
#include "imgui.h"

// The Nexus API handle — defined as a non-static global in dllmain.cpp
extern AddonAPI_t* APIDefs;

// Copy text to the Windows clipboard
void CopyToClipboard(const std::string& text);

// Draw a gradient-backed section header with a colored accent underline.
// Used by equipment panel, clears, and other tabs.
void RenderSectionHeader(const char* label, ImVec4 color, const char* suffix = nullptr);

// Gold-trimmed button matching the RenderSectionHeader primary-action vocabulary.
// Returns true on click. Pass explicit size for fixed-width rows.
bool RenderGoldButton(const char* label, ImVec2 size = ImVec2(0, 0));

// Returns the account name to use for per-account H&S queries.
// Returns an empty string when "All Accounts" is selected.
std::string GetEffectiveAccountName();
