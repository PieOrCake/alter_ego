#include "WikiImage.h"
#include "HttpClient.h"

#include <nlohmann/json.hpp>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

using json = nlohmann::json;

namespace Skinventory {

    AddonAPI_t* WikiImage::s_API = nullptr;
    std::string WikiImage::s_cacheDir;
    std::unordered_map<uint32_t, Texture_t*> WikiImage::s_imageCache;
    std::unordered_map<uint32_t, WikiSkinData> WikiImage::s_wikiDataCache;
    std::unordered_map<uint32_t, bool> WikiImage::s_loading;
    std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> WikiImage::s_failed;
    std::vector<WikiImage::QueuedRequest> WikiImage::s_requestQueue;
    std::vector<uint32_t> WikiImage::s_readyQueue;
    std::mutex WikiImage::s_mutex;
    std::thread WikiImage::s_workerThread;
    std::condition_variable WikiImage::s_queueCV;
    std::atomic<bool> WikiImage::s_stopWorker{false};
    std::unordered_map<std::string, Texture_t*> WikiImage::s_currencyIcons;
    std::vector<std::string> WikiImage::s_currencyReadyQueue;

    void WikiImage::Initialize(AddonAPI_t* api, const std::string& cacheDir) {
        s_API = api;
        s_cacheDir = cacheDir;

        try {
            std::filesystem::create_directories(s_cacheDir);
        } catch (...) {}

        LoadWikiDataCache();

        s_stopWorker = false;
        s_workerThread = std::thread(DownloadWorker);
    }

    void WikiImage::Shutdown() {
        s_stopWorker = true;
        s_queueCV.notify_all();
        if (s_workerThread.joinable()) {
            s_workerThread.join();
        }

        std::lock_guard<std::mutex> lock(s_mutex);
        s_imageCache.clear();
        s_loading.clear();
        s_requestQueue.clear();
        s_readyQueue.clear();
    }

    std::string WikiImage::GetCacheFilePath(uint32_t skinId) {
        return s_cacheDir + "\\" + std::to_string(skinId) + ".jpg";
    }

    std::string WikiImage::GetWikiDataCachePath() {
        return s_cacheDir + "\\wiki_data.json";
    }

    WikiSkinData WikiImage::GetWikiData(uint32_t skinId) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_wikiDataCache.find(skinId);
        if (it != s_wikiDataCache.end()) return it->second;
        return {};
    }

    void WikiImage::SaveWikiDataCache() {
        try {
            json j = json::object();
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                for (const auto& [id, data] : s_wikiDataCache) {
                    json entry;
                    if (!data.acquisition.empty()) entry["acquisition"] = data.acquisition;
                    if (!data.collection.empty()) entry["collection"] = data.collection;
                    if (!data.set_name.empty()) entry["set"] = data.set_name;
                    if (!data.vendor_name.empty()) entry["vendor"] = data.vendor_name;
                    if (!data.vendor_location.empty()) entry["vendor_location"] = data.vendor_location;
                    if (!data.vendor_waypoint.empty()) entry["vendor_waypoint"] = data.vendor_waypoint;
                    if (!data.vendor_cost.empty()) entry["vendor_cost"] = data.vendor_cost;
                    if (!data.vendor_cost_keys.empty()) entry["vendor_cost_keys"] = data.vendor_cost_keys;
                    if (!entry.empty()) j[std::to_string(id)] = entry;
                }
            }
            std::ofstream file(GetWikiDataCachePath());
            if (file.is_open()) file << j.dump();
        } catch (...) {}
    }

    void WikiImage::LoadWikiDataCache() {
        try {
            std::ifstream file(GetWikiDataCachePath());
            if (!file.is_open()) return;
            json j;
            file >> j;
            std::lock_guard<std::mutex> lock(s_mutex);
            for (auto& [key, val] : j.items()) {
                uint32_t id = std::stoul(key);
                WikiSkinData data;
                data.acquisition = val.value("acquisition", "");
                data.collection = val.value("collection", "");
                data.set_name = val.value("set", "");
                data.vendor_name = val.value("vendor", "");
                data.vendor_location = val.value("vendor_location", "");
                data.vendor_waypoint = val.value("vendor_waypoint", "");
                data.vendor_cost = val.value("vendor_cost", "");
                data.vendor_cost_keys = val.value("vendor_cost_keys", "");
                s_wikiDataCache[id] = std::move(data);
            }
        } catch (...) {}
    }

    // Static map of known vendor locations → nearest waypoint chat link
    // Chat codes computed from POI IDs: [0x04, id_le_bytes...] → base64
    static const std::unordered_map<std::string, std::string> s_waypointMap = {
        // Cultural armor vendors
        {"Creator's Commons",   "[&BBQEAAA=]"}, // Magustan Court WP, Rata Sum (id=1044)
        {"Rata Sum",            "[&BBQEAAA=]"},
        {"Trade Commons",       "[&BIYDAAA=]"}, // Trade Commons WP, Hoelbrak (id=902)
        {"Hoelbrak",            "[&BIYDAAA=]"},
        {"Ronan's Bower",       "[&BLwEAAA=]"}, // Ronan's WP, The Grove (id=1212)
        {"The Grove",           "[&BLwEAAA=]"},
        {"Dwayna High Road",    "[&BP4EAAA=]"}, // Ministers WP, Divinity's Reach (id=1278)
        {"Divinity's Reach",    "[&BP4EAAA=]"},
        {"Ligacus Aquilo",      "[&BKkDAAA=]"}, // Ligacus Aquilo WP, Black Citadel (id=937)
        {"Black Citadel",       "[&BKkDAAA=]"},
        // Dungeon vendors
        {"Fort Marriner",       "[&BDAEAAA=]"}, // Fort Marriner WP, Lion's Arch (id=1072)
        {"Lion's Arch",         "[&BDAEAAA=]"},
        // Orr temple karma vendors
        {"Cathedral of Silence",          "[&BCIDAAA=]"}, // Murdered Dreams WP, Cursed Shore (id=802)
        {"Cathedral of Zephyrs",          "[&BLACAAA=]"}, // Tempest WP, Malchor's Leap (id=688)
        {"Cathedral of Glorious Victory", "[&BPoCAAA=]"}, // Glorious Victory WP, Straits of Devastation (id=762)
        {"Cathedral of Verdance",         "[&BBsDAAA=]"}, // Verdance WP, Cursed Shore (id=795)
        {"Cathedral of Eternal Radiance", "[&BK0CAAA=]"}, // Lyssa WP, Malchor's Leap (id=685)
        {"The Narthex",                   "[&BCADAAA=]"}, // Arah WP, Cursed Shore (id=800)
        {"Cursed Shore",                  "[&BCADAAA=]"},
        {"Malchor's Leap",                "[&BLACAAA=]"},
        {"Straits of Devastation",        "[&BPoCAAA=]"},
    };

    // Look up waypoint chat link for a location string
    static std::string LookupWaypoint(const std::string& location) {
        // Try exact match first
        auto it = s_waypointMap.find(location);
        if (it != s_waypointMap.end()) return it->second;
        // Try substring match (location might be "Creator's Commons, Rata Sum")
        for (const auto& [key, val] : s_waypointMap) {
            if (location.find(key) != std::string::npos) return val;
        }
        return "";
    }

    // Parse vendor info from a set page's Acquisition section wikitext.
    // Extracts vendor name(s), location hierarchy from lines like:
    //   * [[Rata Sum]]
    //   ** [[Creator's Commons]]
    //   *** {{vendor|cultural armorsmith}} [[Frinna]]
    // Format a number with comma separators (e.g. 42000 -> "42,000")
    static std::string FormatNumber(int n) {
        std::string s = std::to_string(n);
        int insertPos = (int)s.length() - 3;
        while (insertPos > 0) {
            s.insert(insertPos, ",");
            insertPos -= 3;
        }
        return s;
    }

    // Extract per-piece currency cost from set page wikitext.
    // Handles wiki templates:
    //   {{karma|42000}}              -> "42,000 Karma"
    //   {{token|airship part|500}}   -> "500 Airship Parts"
    //   {{currency|1170|Trade Contract}} -> "1,170 Trade Contracts"
    static std::string ParseCurrencyCost(const std::string& wikitext, std::string& outKeys) {
        struct CurrencyResult {
            int amount = 0;
            std::string name;
            std::string rawKey; // raw currency name for wiki icon lookup
        };
        std::vector<CurrencyResult> found;

        // Helper: extract digits from a string
        auto extractInt = [](const std::string& s) -> int {
            std::string digits;
            for (char c : s) {
                if (c >= '0' && c <= '9') digits += c;
            }
            return digits.empty() ? 0 : std::stoi(digits);
        };

        // Helper: capitalize first letter
        auto capitalize = [](std::string s) -> std::string {
            if (!s.empty() && s[0] >= 'a' && s[0] <= 'z') s[0] -= 32;
            return s;
        };

        // Pattern 1: {{karma|N}} or {{Karma|N}}
        for (const auto& prefix : {"{{karma|", "{{Karma|"}) {
            size_t pos = 0;
            int best = 0;
            while ((pos = wikitext.find(prefix, pos)) != std::string::npos) {
                size_t end = wikitext.find("}}", pos);
                if (end == std::string::npos) break;
                int val = extractInt(wikitext.substr(pos + strlen(prefix), end - pos - strlen(prefix)));
                if (val > 0 && (best == 0 || val < best)) best = val;
                pos = end + 2;
            }
            if (best > 0) found.push_back({best, "Karma", "Karma"});
        }

        // Pattern 2: {{token|NAME|N}} (HoT map currencies)
        {
            const char* prefix = "{{token|";
            size_t pos = 0;
            while ((pos = wikitext.find(prefix, pos)) != std::string::npos) {
                size_t end = wikitext.find("}}", pos);
                if (end == std::string::npos) break;
                std::string inner = wikitext.substr(pos + strlen(prefix), end - pos - strlen(prefix));
                // Split by | -> name, amount
                auto pipe = inner.find('|');
                if (pipe != std::string::npos) {
                    std::string name = inner.substr(0, pipe);
                    int val = extractInt(inner.substr(pipe + 1));
                    if (val > 0 && !name.empty()) {
                        // Capitalize each word for display
                        std::string display;
                        bool newWord = true;
                        for (char c : name) {
                            if (c == ' ') { display += c; newWord = true; }
                            else if (newWord) { display += (char)toupper(c); newWord = false; }
                            else display += c;
                        }
                        // Wiki file: capitalize words, replace spaces with underscores
                        std::string wikiKey = display;
                        std::replace(wikiKey.begin(), wikiKey.end(), ' ', '_');
                        display += "s"; // Pluralize for display
                        found.push_back({val, display, wikiKey});
                    }
                }
                pos = end + 2;
            }
        }

        // Pattern 3: {{currency|N|NAME}} (PoF/LW map currencies)
        {
            const char* prefix = "{{currency|";
            size_t pos = 0;
            while ((pos = wikitext.find(prefix, pos)) != std::string::npos) {
                size_t end = wikitext.find("}}", pos);
                if (end == std::string::npos) break;
                std::string inner = wikitext.substr(pos + strlen(prefix), end - pos - strlen(prefix));
                // Split by | -> amount, name
                auto pipe = inner.find('|');
                if (pipe != std::string::npos) {
                    int val = extractInt(inner.substr(0, pipe));
                    std::string name = inner.substr(pipe + 1);
                    // Trim
                    while (!name.empty() && name.front() == ' ') name.erase(name.begin());
                    while (!name.empty() && name.back() == ' ') name.pop_back();
                    if (val > 0 && !name.empty()) {
                        std::string wikiKey = name;
                        std::replace(wikiKey.begin(), wikiKey.end(), ' ', '_');
                        found.push_back({val, capitalize(name) + "s", wikiKey});
                    }
                }
                pos = end + 2;
            }
        }

        if (found.empty()) return "";

        // Build combined cost string (e.g. "500 Airship Parts + 1g")
        // Pick the smallest occurrence of each currency type (per-piece vs full-set)
        struct BestEntry { int amount; std::string rawKey; };
        std::unordered_map<std::string, BestEntry> best;
        for (const auto& r : found) {
            auto it = best.find(r.name);
            if (it == best.end() || r.amount < it->second.amount) {
                best[r.name] = {r.amount, r.rawKey};
            }
        }

        std::string result;
        std::string keys;
        for (const auto& [name, entry] : best) {
            if (!result.empty()) result += " + ";
            result += FormatNumber(entry.amount) + " " + name;
            if (!keys.empty()) keys += ",";
            keys += entry.rawKey;
        }
        outKeys = keys;
        return result;
    }

    static void ParseSetPageVendors(const std::string& wikitext,
                                     std::string& outVendorName,
                                     std::string& outVendorLocation,
                                     std::string& outWaypoint,
                                     std::string& outVendorCost,
                                     std::string& outVendorCostKeys) {
        // Find the Acquisition section
        auto acqPos = wikitext.find("== Acquisition ==");

        std::string location1, location2, vendorName;

        if (acqPos != std::string::npos) {
        // Parse lines after == Acquisition == until next section or end
        std::istringstream stream(wikitext.substr(acqPos));
        std::string line;
        std::getline(stream, line); // Skip the "== Acquisition ==" line

        while (std::getline(stream, line)) {
            // Stop at next section
            if (line.find("==") == 0) break;

            // Extract [[links]] from the line
            auto extractLink = [](const std::string& s) -> std::string {
                auto start = s.find("[[");
                if (start == std::string::npos) return "";
                start += 2;
                auto end = s.find("]]", start);
                if (end == std::string::npos) return "";
                std::string link = s.substr(start, end - start);
                // Handle piped links like [[Page|Display]]
                auto pipe = link.find('|');
                if (pipe != std::string::npos) link = link.substr(pipe + 1);
                return link;
            };

            // Count leading asterisks to determine hierarchy level
            size_t level = 0;
            for (char c : line) {
                if (c == '*') level++;
                else break;
            }

            if (level == 0) continue;

            std::string link = extractLink(line);
            if (link.empty()) continue;

            bool isVendor = (line.find("{{vendor") != std::string::npos);

            if (isVendor && vendorName.empty()) {
                // Take the first vendor found (PvE), skip subsequent (WvW etc.)
                vendorName = link;
                // Capture the current location context for this vendor
                if (!location2.empty() && !location1.empty()) {
                    outVendorLocation = location2 + ", " + location1;
                } else if (!location1.empty()) {
                    outVendorLocation = location1;
                }
                if (!location2.empty()) outWaypoint = LookupWaypoint(location2);
                if (outWaypoint.empty() && !location1.empty()) outWaypoint = LookupWaypoint(location1);
            } else if (level == 1) {
                location1 = link;
                location2.clear();
            } else if (level == 2) {
                location2 = link;
            }
        }
        } // end if (acqPos != npos)

        outVendorName = vendorName;

        // If no vendor found via Acquisition section, try wiki table format
        // (used by karma vendor set pages like Armageddon armor)
        if (outVendorName.empty()) {
            // Look for a table with Vendor and Location headers
            std::istringstream tableStream(wikitext);
            std::string tline;
            bool inVendorTable = false;
            int vendorCol = -1;
            int locationCol = -1;

            while (std::getline(tableStream, tline)) {
                // Detect table header row with Vendor/Location columns
                if (tline.find('!') == 0 || (tline.find('!') != std::string::npos && tline.find("Vendor") != std::string::npos)) {
                    // Parse column positions from header
                    std::vector<std::string> headers;
                    std::string hline = tline;
                    // Remove leading ! or ||
                    size_t hpos = 0;
                    while (hpos < hline.size() && (hline[hpos] == '!' || hline[hpos] == ' ')) hpos++;
                    hline = hline.substr(hpos);
                    // Split by !!
                    size_t splitPos = 0;
                    int colIdx = 0;
                    while (splitPos != std::string::npos) {
                        size_t nextSplit = hline.find("!!", splitPos);
                        std::string col;
                        if (nextSplit != std::string::npos) {
                            col = hline.substr(splitPos, nextSplit - splitPos);
                            splitPos = nextSplit + 2;
                        } else {
                            col = hline.substr(splitPos);
                            splitPos = std::string::npos;
                        }
                        // Trim
                        while (!col.empty() && col.front() == ' ') col.erase(col.begin());
                        while (!col.empty() && col.back() == ' ') col.pop_back();
                        if (col.find("Vendor") != std::string::npos) vendorCol = colIdx;
                        if (col.find("Location") != std::string::npos) locationCol = colIdx;
                        colIdx++;
                    }
                    if (vendorCol >= 0 && locationCol >= 0) {
                        inVendorTable = true;
                    }
                    continue;
                }

                if (!inVendorTable) continue;

                // End of table
                if (tline.find("|}") == 0) break;
                // Skip row separators
                if (tline.find("|-") == 0) continue;

                // Data row starts with |
                if (tline.size() > 1 && tline[0] == '|' && tline[1] != '-' && tline[1] != '}') {
                    // Split cells by ||
                    std::string dline = tline.substr(1); // skip leading |
                    std::vector<std::string> cells;
                    size_t cpos = 0;
                    while (cpos != std::string::npos) {
                        size_t nextC = dline.find("||", cpos);
                        if (nextC != std::string::npos) {
                            cells.push_back(dline.substr(cpos, nextC - cpos));
                            cpos = nextC + 2;
                        } else {
                            cells.push_back(dline.substr(cpos));
                            cpos = std::string::npos;
                        }
                    }

                    // Extract [[link]] from a cell
                    auto extractCellLink = [](const std::string& cell) -> std::string {
                        auto s = cell.find("[[");
                        if (s == std::string::npos) return "";
                        s += 2;
                        auto e = cell.find("]]", s);
                        if (e == std::string::npos) return "";
                        std::string link = cell.substr(s, e - s);
                        auto pipe = link.find('|');
                        if (pipe != std::string::npos) link = link.substr(pipe + 1);
                        return link;
                    };

                    if (vendorCol < (int)cells.size() && locationCol < (int)cells.size()) {
                        std::string vName = extractCellLink(cells[vendorCol]);
                        std::string vLoc = extractCellLink(cells[locationCol]);
                        if (!vName.empty()) {
                            outVendorName = vName;
                            outVendorLocation = vLoc;
                            outWaypoint = LookupWaypoint(vLoc);
                            break; // Take first vendor
                        }
                    }
                }
            }
        }

        // Extract currency cost (e.g. "42,000 Karma") from the set page text
        outVendorCost = ParseCurrencyCost(wikitext, outVendorCostKeys);
    }

    // Parse Skin infobox fields from wikitext
    static WikiSkinData ParseSkinInfobox(const std::string& wikitext) {
        WikiSkinData data;
        // Extract fields from {{Skin infobox ... }}
        auto extractField = [&](const std::string& field) -> std::string {
            std::string pattern = "| " + field + " = ";
            auto pos = wikitext.find(pattern);
            if (pos == std::string::npos) {
                // Try without space before =
                pattern = "|" + field + "=";
                pos = wikitext.find(pattern);
                if (pos == std::string::npos) return "";
            }
            pos += pattern.size();
            auto end = wikitext.find('\n', pos);
            if (end == std::string::npos) end = wikitext.size();
            std::string val = wikitext.substr(pos, end - pos);
            // Trim whitespace
            while (!val.empty() && (val.back() == ' ' || val.back() == '\r')) val.pop_back();
            while (!val.empty() && val.front() == ' ') val.erase(val.begin());
            // Strip wiki markup like [[ and ]]
            std::string clean;
            for (size_t i = 0; i < val.size(); i++) {
                if (val[i] == '[' || val[i] == ']') continue;
                clean += val[i];
            }
            return clean;
        };

        data.acquisition = extractField("acquisition");
        data.collection = extractField("collection");
        data.set_name = extractField("set");
        return data;
    }

    bool WikiImage::LoadFromDisk(uint32_t skinId) {
        if (!s_API) return false;

        std::string filePath = GetCacheFilePath(skinId);
        DWORD attrs = GetFileAttributesA(filePath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) return false;

        std::string identifier = "WIKI_IMG_" + std::to_string(skinId);
        try {
            Texture_t* tex = s_API->Textures_GetOrCreateFromFile(identifier.c_str(), filePath.c_str());
            if (tex && tex->Resource) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_imageCache[skinId] = tex;
                s_loading.erase(skinId);
                s_failed.erase(skinId);
                return true;
            }
        } catch (...) {}
        return false;
    }

    void WikiImage::RequestImage(uint32_t skinId, const std::string& skinName,
                                     const std::string& weightClass) {
        if (!s_API || skinName.empty()) return;

        {
            std::lock_guard<std::mutex> lock(s_mutex);

            // Already cached or loading
            if (s_imageCache.find(skinId) != s_imageCache.end()) return;
            if (s_loading.find(skinId) != s_loading.end()) return;

            // Check failure cooldown
            auto failIt = s_failed.find(skinId);
            if (failIt != s_failed.end()) {
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - failIt->second).count();
                if (elapsed < RETRY_COOLDOWN_SEC) return;
                s_failed.erase(failIt);
            }

            s_loading[skinId] = true;
        }

        // Try disk cache first
        if (LoadFromDisk(skinId)) return;

        // Queue for download
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_requestQueue.push_back({skinId, skinName, weightClass});
        }
        s_queueCV.notify_one();
    }

    Texture_t* WikiImage::GetImage(uint32_t skinId) {
        if (!s_API) return nullptr;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            auto it = s_imageCache.find(skinId);
            if (it != s_imageCache.end()) {
                if (it->second && it->second->Resource) return it->second;
                s_imageCache.erase(it);
            }
        }

        // Check if Nexus loaded the texture since we last checked
        std::string identifier = "WIKI_IMG_" + std::to_string(skinId);
        Texture_t* tex = nullptr;
        try {
            tex = s_API->Textures_Get(identifier.c_str());
        } catch (...) {
            return nullptr;
        }

        if (tex && tex->Resource) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_imageCache[skinId] = tex;
            s_loading.erase(skinId);
            return tex;
        }

        return nullptr;
    }

    bool WikiImage::IsLoading(uint32_t skinId) {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_loading.find(skinId) != s_loading.end();
    }

    void WikiImage::Tick() {
        if (!s_API) return;

        std::vector<uint32_t> ready;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            ready.swap(s_readyQueue);
        }
        for (uint32_t skinId : ready) {
            try {
                LoadFromDisk(skinId);
            } catch (...) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_loading.erase(skinId);
                s_failed[skinId] = std::chrono::steady_clock::now();
            }
        }

        // Load currency icons that have been downloaded
        std::vector<std::string> currReady;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            currReady.swap(s_currencyReadyQueue);
        }
        for (const auto& key : currReady) {
            try {
                std::string path = CurrencyIconCachePath(key);
                std::string texId = CurrencyIconTexId(key);
                Texture_t* tex = s_API->Textures_GetOrCreateFromFile(texId.c_str(), path.c_str());
                if (tex && tex->Resource) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_currencyIcons[key] = tex;
                }
            } catch (...) {}
        }
    }

    // URL-encode a string for use in wiki API queries
    static std::string UrlEncode(const std::string& str) {
        std::string encoded;
        for (char c : str) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded += c;
            } else if (c == ' ') {
                encoded += '+';
            } else {
                char hex[4];
                snprintf(hex, sizeof(hex), "%%%02X", (unsigned char)c);
                encoded += hex;
            }
        }
        return encoded;
    }

    // Helper: check if a wiki page is a disambiguation or container page via categories.
    static bool IsRedirectPage(const std::string& pageTitle) {
        std::string catUrl = "https://wiki.guildwars2.com/api.php?action=query"
                             "&titles=" + UrlEncode(pageTitle) +
                             "&prop=categories&cllimit=50&format=json";
        std::string catResp = HttpClient::Get(catUrl);
        if (catResp.empty()) return false;

        try {
            json j = json::parse(catResp);
            if (j.contains("query") && j["query"].contains("pages")) {
                for (auto& [pageId, page] : j["query"]["pages"].items()) {
                    if (!page.contains("categories") || !page["categories"].is_array()) continue;
                    for (const auto& cat : page["categories"]) {
                        std::string catTitle = cat.value("title", "");
                        // Lowercase for comparison
                        std::string catLower = catTitle;
                        std::transform(catLower.begin(), catLower.end(), catLower.begin(), ::tolower);
                        if (catLower.find("disambig") != std::string::npos) return true;
                        if (catLower.find("container") != std::string::npos) return true;
                        if (catLower.find("choice") != std::string::npos) return true;
                    }
                }
            }
        } catch (...) {}
        return false;
    }

    // Helper: check if a wiki page exists by direct title. Returns true if page exists (not missing).
    static bool WikiPageExists(const std::string& title) {
        std::string url = "https://wiki.guildwars2.com/api.php?action=query"
                          "&titles=" + UrlEncode(title) +
                          "&format=json";
        std::string resp = HttpClient::Get(url);
        if (resp.empty()) return false;
        try {
            json j = json::parse(resp);
            if (j.contains("query") && j["query"].contains("pages")) {
                for (auto& [pageId, page] : j["query"]["pages"].items()) {
                    // Missing pages have a negative page ID or "missing" key
                    if (pageId == "-1" || page.contains("missing")) return false;
                    return true;
                }
            }
        } catch (...) {}
        return false;
    }

    // Helper: search wiki for a page title by name. Returns empty if not found.
    static std::string WikiSearchPageTitle(const std::string& query) {
        std::string searchUrl = "https://wiki.guildwars2.com/api.php?action=query&list=search"
                                "&srsearch=" + UrlEncode(query) +
                                "&srnamespace=0&srlimit=1&format=json";
        std::string searchResp = HttpClient::Get(searchUrl);
        if (searchResp.empty()) return "";

        try {
            json j = json::parse(searchResp);
            if (j.contains("query") && j["query"].contains("search") &&
                j["query"]["search"].is_array() && !j["query"]["search"].empty()) {
                return j["query"]["search"][0].value("title", "");
            }
        } catch (...) {}
        return "";
    }

    // Helper: find the best skin render image (.jpg preferred) on a wiki page.
    // Returns the File: title of the best image, or empty string if none found.
    static std::string FindBestImageOnPage(const std::string& pageTitle) {
        std::string imagesUrl = "https://wiki.guildwars2.com/api.php?action=query"
                                "&titles=" + UrlEncode(pageTitle) +
                                "&prop=images&imlimit=50&format=json";
        std::string imagesResp = HttpClient::Get(imagesUrl);
        if (imagesResp.empty()) return "";

        std::string bestFile;
        try {
            json j = json::parse(imagesResp);
            if (j.contains("query") && j["query"].contains("pages")) {
                for (auto& [pageId, page] : j["query"]["pages"].items()) {
                    if (!page.contains("images") || !page["images"].is_array()) continue;
                    for (const auto& img : page["images"]) {
                        std::string title = img.value("title", "");
                        if (title.find(".jpg") != std::string::npos ||
                            title.find(".png") != std::string::npos) {
                            // Skip icon files and UI elements
                            if (title.find("Icon") != std::string::npos) continue;
                            if (title.find("icon") != std::string::npos) continue;
                            if (title.find(".png") != std::string::npos && bestFile.empty()) {
                                bestFile = title;
                            }
                            if (title.find(".jpg") != std::string::npos) {
                                bestFile = title;
                                break; // Prefer jpg (usually the full render)
                            }
                        }
                    }
                }
            }
        } catch (...) {}
        return bestFile;
    }

    // Helper: resolve a File: title to its actual download URL.
    static std::string ResolveImageUrl(const std::string& fileTitle) {
        std::string fileUrl = "https://wiki.guildwars2.com/api.php?action=query"
                              "&titles=" + UrlEncode(fileTitle) +
                              "&prop=imageinfo&iiprop=url&format=json";
        std::string fileResp = HttpClient::Get(fileUrl);
        if (fileResp.empty()) return "";

        try {
            json j = json::parse(fileResp);
            if (j.contains("query") && j["query"].contains("pages")) {
                for (auto& [pageId, page] : j["query"]["pages"].items()) {
                    if (page.contains("imageinfo") && page["imageinfo"].is_array() &&
                        !page["imageinfo"].empty()) {
                        return page["imageinfo"][0].value("url", "");
                    }
                }
            }
        } catch (...) {}
        return "";
    }

    WikiSkinData WikiImage::ScrapeAcquisitionFromPage(const std::string& pageTitle) {
        std::string url = "https://wiki.guildwars2.com/api.php?action=parse"
                          "&page=" + UrlEncode(pageTitle) +
                          "&prop=wikitext&format=json";
        std::string resp = HttpClient::Get(url);
        if (resp.empty()) return {};

        try {
            json j = json::parse(resp);
            if (j.contains("parse") && j["parse"].contains("wikitext")) {
                std::string wikitext = j["parse"]["wikitext"].value("*", "");
                if (!wikitext.empty()) {
                    return ParseSkinInfobox(wikitext);
                }
            }
        } catch (...) {}
        return {};
    }

    std::string WikiImage::ScrapeWikiImageUrl(const std::string& skinName,
                                                    const std::string& weightClass,
                                                    std::string& outPageTitle) {
        outPageTitle.clear();
        // Try weight-class-specific skin page first if we have a weight class.
        // These are the most reliable for armor skins (e.g. "Adventurer's Spectacles (heavy skin)")
        if (!weightClass.empty()) {
            std::string wcLower = weightClass;
            std::transform(wcLower.begin(), wcLower.end(), wcLower.begin(), ::tolower);

            // Try "(weight skin)" page first — dedicated skin pages
            std::string skinPageTitle = skinName + " (" + wcLower + " skin)";
            if (WikiPageExists(skinPageTitle)) {
                std::string bestFile = FindBestImageOnPage(skinPageTitle);
                if (!bestFile.empty()) { outPageTitle = skinPageTitle; return ResolveImageUrl(bestFile); }
            }

            // Try "(weight)" page — armor item pages often have the render
            std::string wcPageTitle = skinName + " (" + wcLower + ")";
            if (WikiPageExists(wcPageTitle)) {
                std::string bestFile = FindBestImageOnPage(wcPageTitle);
                if (!bestFile.empty()) { outPageTitle = wcPageTitle; return ResolveImageUrl(bestFile); }
            }
        }

        // Try the direct page title (matches "Open Wiki Page" button behavior)
        if (WikiPageExists(skinName)) {
            // Check if it's a disambig/container — if so, skip it
            if (!IsRedirectPage(skinName)) {
                std::string bestFile = FindBestImageOnPage(skinName);
                if (!bestFile.empty()) { outPageTitle = skinName; return ResolveImageUrl(bestFile); }
            }
        }

        // Last resort: wiki search API
        std::string searchTitle = WikiSearchPageTitle(skinName);
        if (!searchTitle.empty() && searchTitle != skinName) {
            std::string bestFile = FindBestImageOnPage(searchTitle);
            if (!bestFile.empty()) { outPageTitle = searchTitle; return ResolveImageUrl(bestFile); }
        }

        return "";
    }

    std::string WikiImage::CurrencyIconCachePath(const std::string& key) {
        return s_cacheDir + "\\currency_" + key + ".png";
    }

    std::string WikiImage::CurrencyIconTexId(const std::string& key) {
        return "TEX_CURRENCY_" + key;
    }

    Texture_t* WikiImage::GetCurrencyIcon(const std::string& key) {
        if (!s_API || key.empty()) return nullptr;

        {
            std::lock_guard<std::mutex> lock(s_mutex);
            auto it = s_currencyIcons.find(key);
            if (it != s_currencyIcons.end()) {
                if (it->second && it->second->Resource) return it->second;
                s_currencyIcons.erase(it);
            }
        }

        // Check if Nexus loaded the texture since we last checked
        std::string texId = CurrencyIconTexId(key);
        Texture_t* tex = nullptr;
        try {
            tex = s_API->Textures_Get(texId.c_str());
        } catch (...) {
            return nullptr;
        }
        if (tex && tex->Resource) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_currencyIcons[key] = tex;
            return tex;
        }

        // Try loading from disk if cached
        std::string path = CurrencyIconCachePath(key);
        DWORD attrs = GetFileAttributesA(path.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES) {
            try {
                tex = s_API->Textures_GetOrCreateFromFile(texId.c_str(), path.c_str());
                if (tex && tex->Resource) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    s_currencyIcons[key] = tex;
                    return tex;
                }
            } catch (...) {}
        }

        return nullptr;
    }

    void WikiImage::DownloadCurrencyIcons(const std::string& keys) {
        // Split comma-separated keys
        std::vector<std::string> keyList;
        std::istringstream iss(keys);
        std::string k;
        while (std::getline(iss, k, ',')) {
            if (!k.empty()) keyList.push_back(k);
        }

        for (const auto& key : keyList) {
            // Skip if already cached on disk
            std::string path = CurrencyIconCachePath(key);
            DWORD attrs = GetFileAttributesA(path.c_str());
            if (attrs != INVALID_FILE_ATTRIBUTES) {
                // Already on disk, queue for texture load
                std::lock_guard<std::mutex> lock(s_mutex);
                if (s_currencyIcons.find(key) == s_currencyIcons.end()) {
                    s_currencyReadyQueue.push_back(key);
                }
                continue;
            }

            // Query wiki API for icon URL: File:KEY.png
            std::string fileTitle = "File:" + key + ".png";
            std::string imageUrl = ResolveImageUrl(fileTitle);
            if (imageUrl.empty()) continue;

            // Download to cache
            if (HttpClient::DownloadToFile(imageUrl, path)) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_currencyReadyQueue.push_back(key);
            }
        }
    }

    void WikiImage::DownloadWorker() {
        while (!s_stopWorker) {
            QueuedRequest req;

            {
                std::unique_lock<std::mutex> lock(s_mutex);
                s_queueCV.wait_for(lock, std::chrono::milliseconds(200), [] {
                    return s_stopWorker.load() || !s_requestQueue.empty();
                });
                if (s_stopWorker) return;
                if (s_requestQueue.empty()) continue;

                req = s_requestQueue.front();
                s_requestQueue.erase(s_requestQueue.begin());
            }

            // Scrape wiki for image URL and acquisition data
            std::string matchedPage;
            std::string imageUrl = ScrapeWikiImageUrl(req.skinName, req.weightClass, matchedPage);

            // Scrape acquisition data from the matched page (or skin-specific page)
            if (!matchedPage.empty()) {
                WikiSkinData wikiData = ScrapeAcquisitionFromPage(matchedPage);
                // If the matched page is an item page (not a skin page), try the skin page too
                if (wikiData.acquisition.empty() && !req.weightClass.empty()) {
                    std::string wcLower = req.weightClass;
                    std::transform(wcLower.begin(), wcLower.end(), wcLower.begin(), ::tolower);
                    std::string skinPage = req.skinName + " (" + wcLower + " skin)";
                    if (skinPage != matchedPage && WikiPageExists(skinPage)) {
                        wikiData = ScrapeAcquisitionFromPage(skinPage);
                    }
                }

                // If we have a set name, follow to the set page for vendor info
                if (!wikiData.set_name.empty() && wikiData.vendor_name.empty()) {
                    std::string setUrl = "https://wiki.guildwars2.com/api.php?action=parse"
                                          "&page=" + UrlEncode(wikiData.set_name) +
                                          "&prop=wikitext&format=json";
                    std::string setResp = HttpClient::Get(setUrl);
                    if (!setResp.empty()) {
                        try {
                            json sj = json::parse(setResp);
                            if (sj.contains("parse") && sj["parse"].contains("wikitext")) {
                                std::string setWikitext = sj["parse"]["wikitext"].value("*", "");
                                ParseSetPageVendors(setWikitext,
                                    wikiData.vendor_name,
                                    wikiData.vendor_location,
                                    wikiData.vendor_waypoint,
                                    wikiData.vendor_cost,
                                    wikiData.vendor_cost_keys);

                                // Download currency icons for any found currencies
                                if (!wikiData.vendor_cost_keys.empty()) {
                                    DownloadCurrencyIcons(wikiData.vendor_cost_keys);
                                }
                            }
                        } catch (...) {}
                    }
                }

                if (!wikiData.acquisition.empty() || !wikiData.collection.empty() ||
                    !wikiData.set_name.empty() || !wikiData.vendor_name.empty()) {
                    {
                        std::lock_guard<std::mutex> lock(s_mutex);
                        s_wikiDataCache[req.skinId] = wikiData;
                    }
                    SaveWikiDataCache();
                }
            }

            if (imageUrl.empty()) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_loading.erase(req.skinId);
                s_failed[req.skinId] = std::chrono::steady_clock::now();
                continue;
            }

            // Download to disk
            std::string filePath = GetCacheFilePath(req.skinId);
            if (HttpClient::DownloadToFile(imageUrl, filePath)) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_readyQueue.push_back(req.skinId);
            } else {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_loading.erase(req.skinId);
                s_failed[req.skinId] = std::chrono::steady_clock::now();
            }

            // Rate limit
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

}
