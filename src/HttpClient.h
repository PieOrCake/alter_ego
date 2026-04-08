#pragma once

#include <string>
#include <vector>

namespace Skinventory {

    struct HttpResponse {
        int status_code = 0;
        std::string body;
    };

    class HttpClient {
    public:
        // Simple GET returning body only (empty on failure)
        static std::string Get(const std::string& url);

        // GET with full response (status code + body)
        static HttpResponse GetEx(const std::string& url);

        // Download a URL to a file on disk. Returns true on success.
        static bool DownloadToFile(const std::string& url, const std::string& filePath);
    };

}
