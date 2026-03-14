#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

struct temp_GameEntry
{
	std::string name;   // Name of the game
	fs::path gamePath;  // Path of the game executable
};

std::string to_lower(std::string s);
bool isLaunchableFile(const fs::directory_entry& entry);

void scan_directory_for_games(const fs::path& gameDir, std::vector<temp_GameEntry>& outGames);
void sortGamesByName(std::vector<temp_GameEntry>& games);

int launchGame(const fs::path& gamePath);