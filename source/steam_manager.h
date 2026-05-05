#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

struct SteamGame {
	int appid = 0;
	std::string name;
	fs::path libraryPath;
	fs::path installDir;
	fs::path manifestPath;
	long long igdb_id = 0;
};

std::vector<SteamGame> read_installed_steam_games();
bool launch_steam_game_by_appid(int appid);
bool uninstall_steam_game_by_appid(int appid);