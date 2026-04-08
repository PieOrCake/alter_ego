#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <thread>
#include <condition_variable>
#include "nexus/Nexus.h"

namespace Skinventory {

    // Acquisition info scraped from wiki Skin infobox + set page
    struct WikiSkinData {
        std::string acquisition; // e.g. "Obtained from dungeon vendors."
        std::string collection;  // e.g. "Ascalonian Catacombs Collector"
        std::string set_name;    // e.g. "Ascalonian Protector armor"
        std::string vendor_name; // e.g. "Frinna"
        std::string vendor_location; // e.g. "Creator's Commons, Rata Sum"
        std::string vendor_waypoint; // e.g. "[&BBQEAAA=]"
        std::string vendor_cost; // e.g. "42,000 Karma" (per piece, from wiki)
        std::string vendor_cost_keys; // comma-separated raw currency names for icon lookup
    };

    class WikiImage {
    public:
        static void Initialize(AddonAPI_t* api, const std::string& cacheDir);
        static void Shutdown();

        // Request a wiki image for a skin by name (async, on-click)
        // weightClass is used to resolve disambiguation pages (e.g. "heavy", "medium", "light")
        static void RequestImage(uint32_t skinId, const std::string& skinName,
                                  const std::string& weightClass = "");

        // Get loaded texture for a skin (nullptr if not loaded/loading)
        static Texture_t* GetImage(uint32_t skinId);

        // Get wiki acquisition data for a skin (empty if not yet scraped)
        static WikiSkinData GetWikiData(uint32_t skinId);

        // Get a currency icon texture (nullptr if not loaded)
        static Texture_t* GetCurrencyIcon(const std::string& key);

        // Is an image currently being fetched?
        static bool IsLoading(uint32_t skinId);

        // Process ready queue (call every frame from render thread)
        static void Tick();

    private:
        static void DownloadWorker();
        static std::string ScrapeWikiImageUrl(const std::string& skinName,
                                                    const std::string& weightClass,
                                                    std::string& outPageTitle);
        static WikiSkinData ScrapeAcquisitionFromPage(const std::string& pageTitle);
        static std::string GetCacheFilePath(uint32_t skinId);
        static std::string GetWikiDataCachePath();
        static bool LoadFromDisk(uint32_t skinId);
        static void SaveWikiDataCache();
        static void LoadWikiDataCache();

        static AddonAPI_t* s_API;
        static std::string s_cacheDir;

        struct QueuedRequest {
            uint32_t skinId;
            std::string skinName;
            std::string weightClass;
        };

        static std::unordered_map<uint32_t, Texture_t*> s_imageCache;
        static std::unordered_map<uint32_t, WikiSkinData> s_wikiDataCache;
        static std::unordered_map<uint32_t, bool> s_loading;
        static std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> s_failed;
        static const int RETRY_COOLDOWN_SEC = 600;

        static std::vector<QueuedRequest> s_requestQueue;
        static std::vector<uint32_t> s_readyQueue;  // Downloaded, waiting for texture load
        static std::mutex s_mutex;
        static std::thread s_workerThread;
        static std::condition_variable s_queueCV;
        static std::atomic<bool> s_stopWorker;

        // Currency icon cache
        static std::unordered_map<std::string, Texture_t*> s_currencyIcons;
        static std::vector<std::string> s_currencyReadyQueue; // Downloaded, waiting for texture load
        static void DownloadCurrencyIcons(const std::string& keys);
        static std::string CurrencyIconCachePath(const std::string& key);
        static std::string CurrencyIconTexId(const std::string& key);
    };

}
