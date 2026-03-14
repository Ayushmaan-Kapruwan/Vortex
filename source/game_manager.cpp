#include "game_manager.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <system_error>
#include <sstream>

using std::string;
using std::vector;
using std::cerr;

string to_lower(string s) {
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return s;
}

bool isLaunchableFile(const fs::directory_entry& entry)
{
	if (!entry.is_regular_file())
		return false;

	string ext = to_lower(entry.path().extension().string());
	return ext == ".exe";
}

static vector<string> split_tokens_alnum_lower(const string& s)
{
	vector<string> out;
	string cur;
	for (unsigned char ch : s) {
		if (std::isalnum(ch)) cur.push_back(static_cast<char>(std::tolower(ch)));
		else if (!cur.empty()) { out.push_back(cur); cur.clear(); }
	}
	if (!cur.empty()) out.push_back(cur);
	return out;
}

static bool contains_any_substr(const string& hay, const vector<string>& needles)
{
	for (const auto& n : needles) {
		if (hay.find(n) != string::npos) return true;
	}
	return false;
}

static int token_overlap_count(const vector<string>& a, const vector<string>& b)
{
	int c = 0;
	for (const auto& x : a) {
		if (x.size() < 3) continue;
		if (std::find(b.begin(), b.end(), x) != b.end()) ++c;
	}
	return c;
}

void scan_directory_for_games(const fs::path& gameDir, vector<temp_GameEntry>& outGames)
{
	if (!fs::exists(gameDir) || !fs::is_directory(gameDir))
	{
		cerr << "Directory does not exist: " << gameDir << "\n";
		return;
	}

	auto isBadExeName = [](const fs::path& p) -> bool
		{
			string stem = to_lower(p.stem().string()); // exe name without extension

			// Hard reject too-long exe names
			if (stem.size() > 40) return true;

			// Hard reject known non-game executables
			static const vector<string> blocked = {
				"unitycrashhandler",
				"setup",
				"updater",
				"unins",
				"uninstall",
				"vc_redist"
			};

			return contains_any_substr(stem, blocked);
		};

	// Score candidates; higher is better.
	auto score_executable = [&](const fs::path& exePath, const fs::path& gameFolder) -> long long
		{
			const string stem = to_lower(exePath.stem().string());
			const string folderName = to_lower(gameFolder.filename().string());

			const auto exeTokens = split_tokens_alnum_lower(stem);
			const auto folderTokens = split_tokens_alnum_lower(folderName);

			long long score = 0;

			// 1) Prefer name similarity with folder
			const int overlap = token_overlap_count(exeTokens, folderTokens);
			score += static_cast<long long>(overlap) * 1000;

			// Big bonus if full stem appears in folder or vice versa
			if (!stem.empty() && folderName.find(stem) != string::npos) score += 1200;
			if (!folderName.empty() && stem.find(folderName) != string::npos) score += 1200;

			// 2) Penalize suspicious launcher/drm/bootstrap names
			static const vector<string> suspicious = {
				"steam", "uplay", "ubisoft", "launcher", "bootstrap",
				"eac", "easyanticheat", "be", "battleye", "crash", "helper"
			};
			if (contains_any_substr(stem, suspicious)) score -= 1800;

			// 3) Mild preference for larger executable (tie-breaker, not primary)
			std::error_code ec;
			const uintmax_t sz = fs::file_size(exePath, ec);
			if (!ec) {
				// Scale size down so naming quality dominates.
				score += static_cast<long long>(sz / (1024 * 1024)); // +1 per MB
			}

			return score;
		};

	// For each immediate subfolder of gameDir, pick the best scored .exe.
	for (const auto& sub : fs::directory_iterator(gameDir))
	{
		if (!sub.is_directory()) continue;

		const fs::path gameFolder = sub.path();
		const auto folderTokens = split_tokens_alnum_lower(to_lower(gameFolder.filename().string()));

		fs::path bestPath;
		long long bestScore = std::numeric_limits<long long>::min();

		std::error_code ec;
		fs::recursive_directory_iterator it(
			gameFolder,
			fs::directory_options::skip_permission_denied,
			ec
		), end;

		for (; it != end; it.increment(ec))
		{
			if (ec) { ec.clear(); continue; }

			const auto& entry = *it;
			if (!entry.is_regular_file(ec)) { ec.clear(); continue; }

			if (!isLaunchableFile(entry)) continue;
			if (isBadExeName(entry.path())) continue;

			const long long score = score_executable(entry.path(), gameFolder);

			if (bestPath.empty() || score > bestScore)
			{
				bestScore = score;
				bestPath = entry.path();
			}
		}

		if (!bestPath.empty())
		{
			temp_GameEntry g;
			g.name = gameFolder.filename().string();
			g.gamePath = fs::absolute(bestPath);
			outGames.push_back(std::move(g));
		}
	}
}

void sortGamesByName(vector<temp_GameEntry>& games)
{
	std::sort(games.begin(), games.end(),
		[](const temp_GameEntry& a, const temp_GameEntry& b)
		{
			return to_lower(a.name) < to_lower(b.name);
		});
}

int launchGame(const fs::path& gamePath)
{
	// Quote path to handle spaces
	string command = "\"" + gamePath.string() + "\"";
	return std::system(command.c_str());
}