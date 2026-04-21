#include "HttpClient.h"

#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <vector>

namespace Skinventory {

    std::string HttpClient::Get(const std::string& url) {
        HINTERNET hInternet = InternetOpenA("Skinventory/1.0",
            INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return "";

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
                      INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
        if (!hUrl) {
            InternetCloseHandle(hInternet);
            return "";
        }

        std::string result;
        char buffer[8192];
        DWORD bytesRead;
        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            result.append(buffer, bytesRead);
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return result;
    }

    HttpResponse HttpClient::GetEx(const std::string& url) {
        HttpResponse result;
        HINTERNET hInternet = InternetOpenA("Skinventory/1.0",
            INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return result;

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
                      INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
        if (!hUrl) {
            InternetCloseHandle(hInternet);
            return result;
        }

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                       &statusCode, &statusSize, NULL);
        result.status_code = (int)statusCode;

        char buffer[8192];
        DWORD bytesRead;
        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            result.body.append(buffer, bytesRead);
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return result;
    }

    HttpResponse HttpClient::GetEx(const std::string& url, const std::string& headers) {
        HttpResponse result;
        HINTERNET hInternet = InternetOpenA("Skinventory/1.0",
            INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return result;

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
                      INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(),
            headers.c_str(), (DWORD)headers.size(), flags, 0);
        if (!hUrl) {
            InternetCloseHandle(hInternet);
            return result;
        }

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                       &statusCode, &statusSize, NULL);
        result.status_code = (int)statusCode;

        char buffer[8192];
        DWORD bytesRead;
        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            result.body.append(buffer, bytesRead);
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return result;
    }

    HttpResponse HttpClient::PostJson(const std::string& url, const std::string& jsonBody, const std::string& headers) {
        HttpResponse result;
        HINTERNET hInternet = InternetOpenA("Skinventory/1.0",
            INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return result;

        // Parse URL components
        URL_COMPONENTSA uc = {};
        uc.dwStructSize = sizeof(uc);
        char host[256] = {}, path[2048] = {};
        uc.lpszHostName = host; uc.dwHostNameLength = sizeof(host);
        uc.lpszUrlPath = path; uc.dwUrlPathLength = sizeof(path);
        if (!InternetCrackUrlA(url.c_str(), 0, 0, &uc)) {
            InternetCloseHandle(hInternet);
            return result;
        }

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
        if (uc.nScheme == INTERNET_SCHEME_HTTPS)
            flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

        HINTERNET hConnect = InternetConnectA(hInternet, host, uc.nPort,
            NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            return result;
        }

        HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", path, NULL, NULL, NULL, flags, 0);
        if (!hRequest) {
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return result;
        }

        std::string allHeaders = "Content-Type: application/json\r\n" + headers;
        BOOL sent = HttpSendRequestA(hRequest,
            allHeaders.c_str(), (DWORD)allHeaders.size(),
            (LPVOID)jsonBody.c_str(), (DWORD)jsonBody.size());
        if (!sent) {
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return result;
        }

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                       &statusCode, &statusSize, NULL);
        result.status_code = (int)statusCode;

        char buffer[8192];
        DWORD bytesRead;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            result.body.append(buffer, bytesRead);
        }

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return result;
    }

    bool HttpClient::DownloadToFile(const std::string& url, const std::string& filePath) {
        HINTERNET hInternet = InternetOpenA("Skinventory/1.0",
            INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return false;

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
        if (!hUrl) {
            InternetCloseHandle(hInternet);
            return false;
        }

        std::vector<char> buffer;
        char chunk[4096];
        DWORD bytesRead = 0;
        while (InternetReadFile(hUrl, chunk, sizeof(chunk), &bytesRead) && bytesRead > 0) {
            buffer.insert(buffer.end(), chunk, chunk + bytesRead);
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);

        if (buffer.empty()) return false;

        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;
        file.write(buffer.data(), buffer.size());
        file.close();
        return true;
    }

}
