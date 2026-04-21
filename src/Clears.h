#pragma once
#include <atomic>
#include <chrono>
#include <string>

// Event names for H&S responses
#define EV_AE_CLEARS_ACH_RESPONSE "EV_ALTER_EGO_CLEARS_ACH_RESP"
#define EV_AE_VAULT_RESPONSE      "EV_ALTER_EGO_VAULT_RESP"

// Cross-module globals — read/written from dllmain.cpp event handlers and render loop
extern bool g_ClearsFetched;
extern bool g_ClearsFetching;
extern bool g_ClearsNeedRequery;
extern std::atomic<bool> g_ClearsQueryPending;
extern bool g_VaultNeedRequery;

// Public API
void LoadClearsCache();
void LoadVaultCache();
void RenderClears();
void OnClearsAchResponse(void* eventArgs);
void OnVaultResponse(void* eventArgs);

// Reset time helpers (also used by Achievement Tracker persistence)
std::chrono::system_clock::time_point CalcLastDailyReset(std::chrono::system_clock::time_point now);
std::chrono::system_clock::time_point CalcLastWeeklyReset(std::chrono::system_clock::time_point now);
