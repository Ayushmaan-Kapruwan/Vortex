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
#define IGDB_CLIENT_ID "8i3kzr158vqp06bs77j20x0j5mril4" // e.g. "abc123xyz"
#define IGDB_CLIENT_SECRET                                                     \
  "ggu94a410xfkggrkatwga7uhmri1km" // e.g. "supersecretvalue"
// ------------------------------------------------------------

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

using std::cerr;
using std::cout;
using std::string;
using std::wstring;

// ---------- tiny WinHTTP RAII helpers -----------------------

struct HSession {
  HINTERNET h = nullptr;
  ~HSession() {
    if (h)
      WinHttpCloseHandle(h);
  }
};

struct HConnect {
  HINTERNET h = nullptr;
  ~HConnect() {
    if (h)
      WinHttpCloseHandle(h);
  }
};

struct HRequest {
  HINTERNET h = nullptr;
  ~HRequest() {
    if (h)
      WinHttpCloseHandle(h);
  }
};

// Send an HTTPS POST and return the response body as a UTF-8 string.
// host    : e.g. L"id.twitch.tv"
// path    : e.g. L"/oauth2/token"
// headers : additional request headers (may be empty)
// body    : UTF-8 POST body
static string https_post(const wstring &host, const wstring &path,
                         const wstring &headers, const string &body) {
  HSession session;
  HConnect connect;
  HRequest request;

  session.h =
      WinHttpOpen(L"VortexLauncher/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!session.h)
    throw std::runtime_error("WinHttpOpen failed");

  connect.h =
      WinHttpConnect(session.h, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (!connect.h)
    throw std::runtime_error("WinHttpConnect failed");

  request.h = WinHttpOpenRequest(
      connect.h, L"POST", path.c_str(), nullptr, WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
  if (!request.h)
    throw std::runtime_error("WinHttpOpenRequest failed");

  if (!headers.empty())
    WinHttpAddRequestHeaders(request.h, headers.c_str(), (DWORD)headers.size(),
                             WINHTTP_ADDREQ_FLAG_ADD);

  BOOL ok = WinHttpSendRequest(request.h, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                               (LPVOID)body.c_str(), (DWORD)body.size(),
                               (DWORD)body.size(), 0);
  if (!ok)
    throw std::runtime_error("WinHttpSendRequest failed");

  if (!WinHttpReceiveResponse(request.h, nullptr))
    throw std::runtime_error("WinHttpReceiveResponse failed");

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

// ---------- token cache -------------------------------------

static string s_access_token; // cached for the process lifetime

static bool igdb_fetch_token() {
  const string client_id = IGDB_CLIENT_ID;
  const string client_secret = IGDB_CLIENT_SECRET;

  if (client_id.empty() || client_secret.empty())
    return false;

  const string body = "client_id=" + client_id +
                      "&client_secret=" + client_secret +
                      "&grant_type=client_credentials";

  string resp;
  try {
    resp = https_post(L"id.twitch.tv", L"/oauth2/token", L"", body);
  } catch (const std::exception &e) {
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
  while (pos < resp.size() && resp[pos] == ' ')
    ++pos;
  if (pos >= resp.size() || resp[pos] != ':')
    return false;
  ++pos;
  // skip whitespace, then opening '"'
  while (pos < resp.size() && resp[pos] == ' ')
    ++pos;
  if (pos >= resp.size() || resp[pos] != '"')
    return false;
  ++pos;

  auto end = resp.find('"', pos);
  if (end == string::npos)
    return false;

  s_access_token = resp.substr(pos, end - pos);
  return !s_access_token.empty();
}

// ---------- name extraction ---------------------------------

// Extract one game info object starting the search from position `from`.
// Returns the info and advances `from` past the object. Returns empty string
// for name when no more.
static IgdbGameInfo extract_info_at(const string &json, size_t &from) {
  IgdbGameInfo info;

  auto start_obj = json.find('{', from);
  if (start_obj == string::npos) {
    from = string::npos;
    return info;
  }

  auto end_obj = json.find('}', start_obj);
  if (end_obj == string::npos) {
    from = string::npos;
    return info;
  }

  from = end_obj + 1;
  string obj = json.substr(start_obj, end_obj - start_obj);

  // Extract "id"
  auto id_pos = obj.find("\"id\"");
  if (id_pos != string::npos) {
    id_pos += 4;
    while (id_pos < obj.size() && (obj[id_pos] == ' ' || obj[id_pos] == ':'))
      id_pos++;
    long long id = 0;
    while (id_pos < obj.size() && std::isdigit((unsigned char)obj[id_pos])) {
      id = id * 10 + (obj[id_pos] - '0');
      id_pos++;
    }
    info.id = id;
  }

  // Extract "name"
  auto name_pos = obj.find("\"name\"");
  if (name_pos != string::npos) {
    name_pos += 6;
    while (name_pos < obj.size() &&
           (obj[name_pos] == ' ' || obj[name_pos] == ':'))
      name_pos++;
    if (name_pos < obj.size() && obj[name_pos] == '"') {
      name_pos++;
      bool escaped = false;
      for (; name_pos < obj.size(); ++name_pos) {
        char c = obj[name_pos];
        if (escaped) {
          info.name += c;
          escaped = false;
        } else if (c == '\\') {
          escaped = true;
        } else if (c == '"') {
          break;
        } else {
          info.name += c;
        }
      }
    }
  }

  return info;
}

// Walk all "name" fields in the response.
// Priority:
//   1. Canonical exact match  (alphanumeric-only, lowercase)
//   2. First result as fallback
static IgdbGameInfo extract_best_info(const string &json, const string &query) {
  const string queryCanon = make_canonical(query);
  IgdbGameInfo first;
  size_t cursor = 0;

  while (cursor != string::npos) {
    IgdbGameInfo info = extract_info_at(json, cursor);
    if (info.name.empty())
      break;
    if (first.name.empty())
      first = info;
    if (make_canonical(info.name) == queryCanon)
      return info; // canonical match — wins immediately
  }
  return first; // no canonical match — best guess is first result
}

// ---------- Cache -------------------------------------------

static std::unordered_map<std::string, IgdbGameInfo> s_igdb_cache;
static bool s_cache_loaded = false;
static const std::filesystem::path IGDB_CACHE_FILE = "igdb_cache.txt";

static void load_igdb_cache() {
  if (s_cache_loaded)
    return;
  s_cache_loaded = true;

  if (!std::filesystem::exists(IGDB_CACHE_FILE))
    return;

  std::ifstream file(IGDB_CACHE_FILE);
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue;

    auto eq_pos = line.find('=');
    if (eq_pos != std::string::npos) {
      std::string query = line.substr(0, eq_pos);
      std::string rest = line.substr(eq_pos + 1);

      auto pipe_pos = rest.find('|');
      if (pipe_pos != std::string::npos) {
        try {
          IgdbGameInfo info;
          info.id = std::stoll(rest.substr(0, pipe_pos));
          info.name = rest.substr(pipe_pos + 1);
          s_igdb_cache[query] = info;
        } catch (...) {
        }
      }
    }
  }
}

static void save_to_cache(const std::string &query, const IgdbGameInfo &info) {
  s_igdb_cache[query] = info;
  std::ofstream file(IGDB_CACHE_FILE, std::ios::app);
  if (file.is_open()) {
    file.seekp(0, std::ios::end);
    if (file.tellp() == 0) {
      file << "# IGDB Resolution Cache\n";
      file << "# Format: SEARCH_QUERY=IGDB_ID|CANONICAL_NAME\n";
    }
    file << query << "=" << info.id << "|" << info.name << "\n";
  }
}

// ---------- public API --------------------------------------

IgdbGameInfo igdb_resolve_game(const std::string &folderName,
                               bool interactive) {
  load_igdb_cache();

  // Check cache first
  auto it = s_igdb_cache.find(folderName);
  if (it != s_igdb_cache.end()) {
    return it->second;
  }

  IgdbGameInfo fallback;
  fallback.name = folderName;
  fallback.id = 0;

  // Guard: no credentials
  const string client_id = IGDB_CLIENT_ID;
  if (client_id.empty()) {
    cerr << "[IGDB] No credentials set — skipping lookup.\n";
    save_to_cache(folderName, fallback);
    return fallback;
  }

  // Fetch token once per process
  if (s_access_token.empty()) {
    if (!igdb_fetch_token())
      return fallback;
  }

  // Build query body (Apicalypse syntax)
  // Escape double-quotes inside folderName just in case
  string safe = folderName;
  for (size_t i = 0; i < safe.size(); ++i)
    if (safe[i] == '"') {
      safe.insert(i, "\\");
      ++i;
    }

  const string body = "search \"" + safe + "\"; fields id, name; limit 5;";

  // Build headers
  const wstring headers =
      L"Client-ID: " + wstring(client_id.begin(), client_id.end()) + L"\r\n" +
      L"Authorization: Bearer " +
      wstring(s_access_token.begin(), s_access_token.end()) + L"\r\n" +
      L"Content-Type: text/plain";

  string resp;
  try {
    resp = https_post(L"api.igdb.com", L"/v4/games", headers, body);
  } catch (const std::exception &e) {
    cerr << "[IGDB] API call failed: " << e.what() << "\n";
    save_to_cache(folderName, fallback);
    return fallback;
  }

  // cerr << "[IGDB] Raw response for \"" << folderName << "\": " << resp <<
  // "\n";

  IgdbGameInfo resolved = extract_best_info(resp, folderName);

  if (resolved.name.empty()) {
    if (!interactive) {
      save_to_cache(folderName, fallback);
      return fallback;
    }

    // No match — ask the user
    cout << "\n[IGDB] No match found for \"" << folderName << "\"\n";
    cout << "  [1] Keep folder name\n";
    cout << "  [2] Enter name manually\n";
    cout << "Enter choice: ";

    int choice = -1;
    if (!(std::cin >> choice)) {
      std::cin.clear();
      std::cin.ignore(10000, '\n');
      save_to_cache(folderName, fallback);
      return fallback;
    }

    if (choice == 2) {
      std::cout << "Enter canonical name: ";
      std::cin.ignore();
      std::getline(std::cin, fallback.name);
      fallback.id = 0; // manual entry implies no IGDB ID
    }

    save_to_cache(folderName, fallback);
    return fallback;
  }

  // std::cerr << "[IGDB] \"" << folderName << "\" -> \"" << resolved.name <<
  // "\" (ID: " << resolved.id << ")\n";
  save_to_cache(folderName, resolved);
  return resolved;
}
