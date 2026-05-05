#include "steamgriddb_manager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

using std::string;
using std::wstring;
namespace fs = std::filesystem;

#define STEAMGRIDDB_API_KEY "0cd8c47db2582b99a47c3aa7ca700448"

struct HSession { HINTERNET h = nullptr; ~HSession() { if (h) WinHttpCloseHandle(h); } };
struct HConnect { HINTERNET h = nullptr; ~HConnect() { if (h) WinHttpCloseHandle(h); } };
struct HRequest { HINTERNET h = nullptr; ~HRequest() { if (h) WinHttpCloseHandle(h); } };

static string https_get(const wstring &host, const wstring &path, const wstring &headers) {
    HSession session;
    HConnect connect;
    HRequest request;

    session.h = WinHttpOpen(L"VortexLauncher/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session.h) throw std::runtime_error("WinHttpOpen failed");

    connect.h = WinHttpConnect(session.h, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connect.h) throw std::runtime_error("WinHttpConnect failed");

    request.h = WinHttpOpenRequest(connect.h, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request.h) throw std::runtime_error("WinHttpOpenRequest failed");

    if (!headers.empty()) {
        WinHttpAddRequestHeaders(request.h, headers.c_str(), (DWORD)headers.size(), WINHTTP_ADDREQ_FLAG_ADD);
    }

    BOOL ok = WinHttpSendRequest(request.h, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!ok) throw std::runtime_error("WinHttpSendRequest failed");

    if (!WinHttpReceiveResponse(request.h, nullptr)) throw std::runtime_error("WinHttpReceiveResponse failed");

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(request.h, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
    if (statusCode != 200) {
        throw std::runtime_error("HTTP Error " + std::to_string(statusCode));
    }

    string result;
    DWORD available = 0;
    while (WinHttpQueryDataAvailable(request.h, &available) && available > 0) {
        string chunk(available, '\0');
        DWORD read = 0;
        WinHttpReadData(request.h, &chunk[0], available, &read);
        chunk.resize(read);
        result += chunk;
    }
    return result;
}

static bool download_file(const string& url, const fs::path& dest_path) {
    if (url.substr(0, 8) != "https://") return false;
    size_t host_end = url.find('/', 8);
    if (host_end == string::npos) return false;

    string host_str = url.substr(8, host_end - 8);
    string path_str = url.substr(host_end);

    wstring host(host_str.begin(), host_str.end());
    wstring path(path_str.begin(), path_str.end());

    try {
        string data = https_get(host, path, L"");
        if (data.empty()) return false;
        
        std::ofstream out(dest_path, std::ios::binary);
        if (!out) return false;
        out.write(data.data(), data.size());
        return true;
    } catch (...) {
        return false;
    }
}

static string extract_json_value(const string& json, const string& key, bool is_number = false) {
    string search_key = "\"" + key + "\":";
    size_t pos = json.find(search_key);
    if (pos == string::npos) return "";
    pos += search_key.length();

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) pos++;

    if (is_number) {
        size_t end = pos;
        while (end < json.size() && std::isdigit((unsigned char)json[end])) end++;
        return json.substr(pos, end - pos);
    } else {
        if (pos < json.size() && json[pos] == '"') {
            pos++; // skip quote
            size_t end = pos;
            bool escaped = false;
            string val;
            while (end < json.size()) {
                if (escaped) {
                    val += json[end];
                    escaped = false;
                } else if (json[end] == '\\') {
                    escaped = true;
                } else if (json[end] == '"') {
                    break;
                } else {
                    val += json[end];
                }
                end++;
            }
            string unescaped;
            for (size_t i = 0; i < val.size(); ++i) {
                if (val[i] == '\\' && i + 1 < val.size() && val[i+1] == '/') {
                    unescaped += '/';
                    ++i;
                } else {
                    unescaped += val[i];
                }
            }
            return unescaped;
        }
    }
    return "";
}

static string url_encode(const string& value) {
    string escaped;
    for (char c : value) {
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped += c;
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            escaped += buf;
        }
    }
    return escaped;
}

static bool is_dir_empty(const fs::path& p) {
    if (!fs::exists(p) || !fs::is_directory(p)) return true;
    std::error_code ec;
    return fs::is_empty(p, ec);
}

void ensure_steamgriddb_images(const std::vector<std::string>& game_names) {
    string api_key = STEAMGRIDDB_API_KEY;
    if (api_key == "YOUR_API_KEY_HERE" || api_key.empty()) {
        std::cerr << "[SteamGridDB] No API Key set. Skipping downloads.\n";
        return;
    }

    wstring headers = L"Authorization: Bearer " + wstring(api_key.begin(), api_key.end());

    for (const auto& name : game_names) {
        // Sanitize name for folder creation
        string safe_name = name;
        for (char& c : safe_name) {
            if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*') {
                c = '_';
            }
        }

        fs::path game_img_dir = fs::path("Images") / (safe_name + "_img");
        fs::path grid_dir = game_img_dir / "grid";
        fs::path logo_dir = game_img_dir / "logo";
        fs::path hero_dir = game_img_dir / "hero";

        bool need_grid = is_dir_empty(grid_dir);
        bool need_logo = is_dir_empty(logo_dir);
        bool need_hero = is_dir_empty(hero_dir);

        if (!need_grid && !need_logo && !need_hero) {
            continue; // Skip if all folders have content
        }

        std::cout << "[SteamGridDB] Fetching art for: " << name << "...\n";

        string search_url_path = "/api/v2/search/autocomplete/" + url_encode(name);
        string search_resp;
        try {
            search_resp = https_get(L"www.steamgriddb.com", wstring(search_url_path.begin(), search_url_path.end()), headers);
        } catch (...) {
            std::cerr << "  -> Search failed for " << name << "\n";
            continue;
        }

        string id_str = extract_json_value(search_resp, "id", true);
        if (id_str.empty()) {
            std::cerr << "  -> No match found for " << name << "\n";
            continue;
        }

        fs::create_directories(grid_dir);
        fs::create_directories(logo_dir);
        fs::create_directories(hero_dir);

        if (need_grid) {
            string grids_path = "/api/v2/grids/game/" + id_str + "?dimensions=600x900";
            try {
                string grids_resp = https_get(L"www.steamgriddb.com", wstring(grids_path.begin(), grids_path.end()), headers);
                string grid_url = extract_json_value(grids_resp, "url");
                if (!grid_url.empty()) {
                    string ext = ".png";
                    if (grid_url.find(".jpg") != string::npos) ext = ".jpg";
                    else if (grid_url.find(".jpeg") != string::npos) ext = ".jpeg";
                    
                    fs::path file_path = grid_dir / ("grid" + ext);
                    if (download_file(grid_url, file_path)) {
                        std::cout << "  -> Downloaded grid\n";
                    }
                }
            } catch (...) {}
        }

        if (need_logo) {
            string logos_path = "/api/v2/logos/game/" + id_str;
            try {
                string logos_resp = https_get(L"www.steamgriddb.com", wstring(logos_path.begin(), logos_path.end()), headers);
                string logo_url = extract_json_value(logos_resp, "url");
                if (!logo_url.empty()) {
                    string ext = ".png";
                    if (logo_url.find(".jpg") != string::npos) ext = ".jpg";
                    fs::path file_path = logo_dir / ("logo" + ext);
                    if (download_file(logo_url, file_path)) {
                        std::cout << "  -> Downloaded logo\n";
                    }
                }
            } catch (...) {}
        }

        if (need_hero) {
            string heroes_path = "/api/v2/heroes/game/" + id_str;
            try {
                string heroes_resp = https_get(L"www.steamgriddb.com", wstring(heroes_path.begin(), heroes_path.end()), headers);
                string hero_url = extract_json_value(heroes_resp, "url");
                if (!hero_url.empty()) {
                    string ext = ".png";
                    if (hero_url.find(".jpg") != string::npos) ext = ".jpg";
                    fs::path file_path = hero_dir / ("hero" + ext);
                    if (download_file(hero_url, file_path)) {
                        std::cout << "  -> Downloaded hero\n";
                    }
                }
            } catch (...) {}
        }
    }
}
