#include "steam_manager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <windows.h>
#include <shellapi.h>
#include <string>

#include "igdb_manager.h"

using std::string;
using std::vector;

static string trim(const string& s) {
	size_t a = 0, b = s.size();
	while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
	while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
	return s.substr(a, b - a);
}

static string read_text_file(const fs::path& p) {
	std::ifstream in(p, std::ios::binary);
	if (!in) return {};
	std::ostringstream ss;
	ss << in.rdbuf();
	return ss.str();
}

static bool get_steam_path_from_registry(fs::path& outSteamPath) {
	HKEY hKey = nullptr;
	const char* subKey = "Software\\Valve\\Steam";
	if (RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		return false;
	}

	char buffer[4096];
	DWORD bufferSize = sizeof(buffer);
	DWORD type = 0;
	LONG rc = RegQueryValueExA(hKey, "SteamPath", nullptr, &type,
		reinterpret_cast<LPBYTE>(buffer), &bufferSize);
	RegCloseKey(hKey);

	if (rc != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ) || bufferSize == 0) {
		return false;
	}

	if (bufferSize >= sizeof(buffer)) buffer[sizeof(buffer) - 1] = '\0';
	else buffer[bufferSize] = '\0';

	outSteamPath = fs::path(buffer);
	return !outSteamPath.empty();
}

static vector<fs::path> parse_libraryfolders_vdf(const fs::path& steamPath) {
	vector<fs::path> libs;

	fs::path vdfPath = steamPath / "steamapps" / "libraryfolders.vdf";
	string txt = read_text_file(vdfPath);
	if (txt.empty()) return libs;

	std::regex pathRe("\"path\"\\s*\"([^\"]+)\"");
		auto begin = std::sregex_iterator(txt.begin(), txt.end(), pathRe);
	auto end = std::sregex_iterator();

	std::unordered_set<string> seen;
	for (auto it = begin; it != end; ++it) {
		string raw = (*it)[1].str();

		string unescaped;
		unescaped.reserve(raw.size());
		for (size_t i = 0; i < raw.size(); ++i) {
			if (raw[i] == '\\' && i + 1 < raw.size() && raw[i + 1] == '\\') {
				unescaped.push_back('\\');
				++i;
			}
			else {
				unescaped.push_back(raw[i]);
			}
		}

		fs::path p = fs::path(unescaped);
		string key = p.string();
		if (!key.empty() && !seen.count(key)) {
			seen.insert(key);
			libs.push_back(p);
		}
	}

	string steamKey = steamPath.string();
	if (!steamKey.empty() && !seen.count(steamKey)) {
		libs.push_back(steamPath);
	}

	return libs;
}

static bool extract_acf_field(const string& acf, const string& key, string& outValue) {
	std::regex re("\"" + key + "\"\\s*\"([^\"]*)\"");
		std::smatch m;
	if (std::regex_search(acf, m, re) && m.size() >= 2) {
		outValue = m[1].str();
		return true;
	}
	return false;
}

static bool parse_appmanifest(const fs::path& manifestPath, SteamGame& outGame) {
	string acf = read_text_file(manifestPath);
	if (acf.empty()) return false;

	string appidStr, name, installdir;
	if (!extract_acf_field(acf, "appid", appidStr)) return false;
	if (!extract_acf_field(acf, "name", name)) name.clear();
	if (!extract_acf_field(acf, "installdir", installdir)) installdir.clear();

	try {
		outGame.appid = std::stoi(trim(appidStr));
	}
	catch (...) {
		return false;
	}

	outGame.name = name;
	outGame.manifestPath = manifestPath;
	return true;
}

vector<SteamGame> read_installed_steam_games() {
	vector<SteamGame> games;

	fs::path steamPath;
	if (!get_steam_path_from_registry(steamPath)) {
		std::cerr << "Could not find SteamPath in registry.\n";
		return games;
	}

	vector<fs::path> libraries = parse_libraryfolders_vdf(steamPath);
	if (libraries.empty()) {
		std::cerr << "No Steam libraries found.\n";
		return games;
	}

	std::unordered_set<int> seenAppIds;

	for (const auto& lib : libraries) {
		fs::path steamapps = lib / "steamapps";
		if (!fs::exists(steamapps) || !fs::is_directory(steamapps)) continue;

		for (const auto& e : fs::directory_iterator(steamapps)) {
			if (!e.is_regular_file()) continue;

			const fs::path p = e.path();
			const string fn = p.filename().string();

			if (fn.rfind("appmanifest_", 0) != 0) continue;
			if (p.extension() != ".acf") continue;

			SteamGame g;
			if (!parse_appmanifest(p, g)) continue;

			if (seenAppIds.count(g.appid)) continue;
			seenAppIds.insert(g.appid);

			g.libraryPath = lib;

			string acf = read_text_file(p);
			string installdir;
			if (extract_acf_field(acf, "installdir", installdir) && !installdir.empty()) {
				g.installDir = lib / "steamapps" / "common" / installdir;
			}

			IgdbGameInfo info = igdb_resolve_game(g.name, false);
			g.igdb_id = info.id;

			games.push_back(std::move(g));
		}
	}

	std::sort(games.begin(), games.end(), [](const SteamGame& a, const SteamGame& b) {
		if (a.name == b.name) return a.appid < b.appid;
		return a.name < b.name;
		});

	return games;
}

bool launch_steam_game_by_appid(int appid) {
	string uri = "steam://run/" + std::to_string(appid);
	HINSTANCE res = ShellExecuteA(nullptr, "open", uri.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	return reinterpret_cast<intptr_t>(res) > 32;
}