// ============================================================
//  igdb_manager.cpp
//  Resolves local game folder names to canonical IGDB names.
//
//  Setup: paste your Twitch developer credentials below.
//  Get them at https://dev.twitch.tv/console (free, needs 2FA).
// ============================================================

#include "igdb_manager.h"
#include "game_manager.h"

// ---- Paste your credentials here ---------------------------
#define IGDB_CLIENT_ID     "8i3kzr158vqp06bs77j20x0j5mril4"   // e.g. "abc123xyz"
#define IGDB_CLIENT_SECRET "ggu94a410xfkggrkatwga7uhmri1km"   // e.g. "supersecretvalue"
// ------------------------------------------------------------

#include <iostream>
#include <string>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

using std::string;
using std::wstring;
using std::cerr;
using std::cout;

// ---------- tiny WinHTTP RAII helpers -----------------------

struct HSession {
    HINTERNET h = nullptr;
    ~HSession() { if (h) WinHttpCloseHandle(h); }
};

struct HConnect {
    HINTERNET h = nullptr;
    ~HConnect() { if (h) WinHttpCloseHandle(h); }
};

struct HRequest {
    HINTERNET h = nullptr;
    ~HRequest() { if (h) WinHttpCloseHandle(h); }
};

// Send an HTTPS POST and return the response body as a UTF-8 string.
// host    : e.g. L"id.twitch.tv"
// path    : e.g. L"/oauth2/token"
// headers : additional request headers (may be empty)
// body    : UTF-8 POST body
static string https_post(const wstring& host,
                         const wstring& path,
                         const wstring& headers,
                         const string&  body)
{
    HSession  session;
    HConnect  connect;
    HRequest  request;

    session.h = WinHttpOpen(L"VortexLauncher/1.0",
                            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                            WINHTTP_NO_PROXY_NAME,
                            WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session.h) throw std::runtime_error("WinHttpOpen failed");

    connect.h = WinHttpConnect(session.h, host.c_str(),
                               INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connect.h) throw std::runtime_error("WinHttpConnect failed");

    request.h = WinHttpOpenRequest(connect.h, L"POST", path.c_str(),
                                   nullptr, WINHTTP_NO_REFERER,
                                   WINHTTP_DEFAULT_ACCEPT_TYPES,
                                   WINHTTP_FLAG_SECURE);
    if (!request.h) throw std::runtime_error("WinHttpOpenRequest failed");

    if (!headers.empty())
        WinHttpAddRequestHeaders(request.h, headers.c_str(),
                                 (DWORD)headers.size(),
                                 WINHTTP_ADDREQ_FLAG_ADD);

    BOOL ok = WinHttpSendRequest(request.h,
                                 WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 (LPVOID)body.c_str(), (DWORD)body.size(),
                                 (DWORD)body.size(), 0);
    if (!ok) throw std::runtime_error("WinHttpSendRequest failed");

    if (!WinHttpReceiveResponse(request.h, nullptr))
        throw std::runtime_error("WinHttpReceiveResponse failed");

    string result;
    DWORD  available = 0;
    while (WinHttpQueryDataAvailable(request.h, &available) && available > 0) {
        string chunk(available, '\0');
        DWORD  read = 0;
        WinHttpReadData(request.h, &chunk[0], available, &read);
        chunk.resize(read);
        result += chunk;
    }
    return result;
}

// ---------- token cache -------------------------------------

static string s_access_token;   // cached for the process lifetime

static bool igdb_fetch_token()
{
    const string client_id     = IGDB_CLIENT_ID;
    const string client_secret = IGDB_CLIENT_SECRET;

    if (client_id.empty() || client_secret.empty())
        return false;

    const string body =
        "client_id="     + client_id +
        "&client_secret=" + client_secret +
        "&grant_type=client_credentials";

    string resp;
    try {
        resp = https_post(L"id.twitch.tv", L"/oauth2/token", L"", body);
    } catch (const std::exception& e) {
        cerr << "[IGDB] Token fetch failed: " << e.what() << "\n";
        return false;
    }

    // Extract "access_token": "<value>" from JSON.
    // Handles both compact (no space) and pretty (space after colon) formats.
    const string key = "\"access_token\"";
    auto pos = resp.find(key);
    if (pos == string::npos) {
        cerr << "[IGDB] Unexpected token response: " << resp << "\n";
        return false;
    }
    pos += key.size();
    // skip whitespace, then ':'
    while (pos < resp.size() && resp[pos] == ' ') ++pos;
    if (pos >= resp.size() || resp[pos] != ':') return false;
    ++pos;
    // skip whitespace, then opening '"'
    while (pos < resp.size() && resp[pos] == ' ') ++pos;
    if (pos >= resp.size() || resp[pos] != '"') return false;
    ++pos;

    auto end = resp.find('"', pos);
    if (end == string::npos) return false;

    s_access_token = resp.substr(pos, end - pos);
    return !s_access_token.empty();
}

// ---------- name extraction ---------------------------------

// Extract one "name" value starting the search from position `from`.
// Returns the name and advances `from` past it. Returns "" when no more.
static string extract_name_at(const string& json, size_t& from)
{
    const string key = "\"name\"";
    auto pos = json.find(key, from);
    if (pos == string::npos) { from = string::npos; return ""; }
    from = pos + key.size();

    // skip whitespace, then ':'
    while (from < json.size() && json[from] == ' ') ++from;
    if (from >= json.size() || json[from] != ':') { from = string::npos; return ""; }
    ++from;
    // skip whitespace, then opening '"'
    while (from < json.size() && json[from] == ' ') ++from;
    if (from >= json.size() || json[from] != '"') { from = string::npos; return ""; }
    ++from;

    string name;
    bool escaped = false;
    for (; from < json.size(); ++from) {
        char c = json[from];
        if (escaped) { name += c; escaped = false; }
        else if (c == '\\') { escaped = true; }
        else if (c == '"') { ++from; break; }
        else { name += c; }
    }
    return name;
}

// Walk all "name" fields in the response.
// Priority:
//   1. Canonical exact match  (alphanumeric-only, lowercase)
//   2. First result as fallback
static string extract_best_name(const string& json, const string& query)
{
    const string queryCanon = make_canonical(query);
    string first;
    size_t cursor = 0;

    while (cursor != string::npos) {
        string name = extract_name_at(json, cursor);
        if (name.empty()) break;
        if (first.empty()) first = name;
        if (make_canonical(name) == queryCanon)
            return name;   // canonical match — wins immediately
    }
    return first;          // no canonical match — best guess is first result
}

// ---------- public API --------------------------------------

std::string igdb_resolve_name(const std::string& folderName)
{
    // Guard: no credentials
    const string client_id = IGDB_CLIENT_ID;
    if (client_id.empty()) {
        cerr << "[IGDB] No credentials set — skipping lookup.\n";
        return folderName;
    }

    // Fetch token once per process
    if (s_access_token.empty()) {
        if (!igdb_fetch_token())
            return folderName;
    }

    // Build query body (Apicalypse syntax)
    // Escape double-quotes inside folderName just in case
    string safe = folderName;
    for (size_t i = 0; i < safe.size(); ++i)
        if (safe[i] == '"') { safe.insert(i, "\\"); ++i; }

    const string body = "search \"" + safe + "\"; fields name; limit 5;";

    // Build headers
    const wstring headers =
        L"Client-ID: "    + wstring(client_id.begin(), client_id.end()) + L"\r\n" +
        L"Authorization: Bearer " +
            wstring(s_access_token.begin(), s_access_token.end()) + L"\r\n" +
        L"Content-Type: text/plain";

    string resp;
    try {
        resp = https_post(L"api.igdb.com", L"/v4/games", headers, body);
    } catch (const std::exception& e) {
        cerr << "[IGDB] API call failed: " << e.what() << "\n";
        return folderName;
    }

    cerr << "[IGDB] Raw response for \"" << folderName << "\": " << resp << "\n";

    string resolved = extract_best_name(resp, folderName);

    if (resolved.empty()) {
        // No match — ask the user
        cout << "\n[IGDB] No match found for \"" << folderName << "\"\n";
        cout << "  [1] Keep folder name\n";
        cout << "  [2] Enter name manually\n";
        cout << "Enter choice: ";

        int choice = -1;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 2) {
            cout << "Enter name: ";
            string manual;
            std::getline(std::cin, manual);
            if (!manual.empty())
                return manual;
        }
        return folderName;
    }

    cerr << "[IGDB] \"" << folderName << "\" -> \"" << resolved << "\"\n";
    return resolved;
}
