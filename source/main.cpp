#include <ctime>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "game_manager.h"
#include "igdb_manager.h"
#include "metadata_manager.h"
#include "preference_manager.h"
#include "stats_manager.h"
#include "steam_manager.h"
#include "steamgriddb_manager.h"

#include <chrono>
#include <thread>

namespace fs = std::filesystem;

using std::cin;
using std::cout;

enum class UserMood { Relaxed = 0, Competitive = 1, Immersive = 2 };

static UserMood ask_user_mood() {
  cout << "\n=== Select Your Mood ===\n";
  cout << "  [0] Relaxed\n";
  cout << "  [1] Competitive\n";
  cout << "  [2] Immersive\n\n";

  while (true) {
    cout << "Enter mood (0-2): ";
    int choice = -1;
    if (!(cin >> choice) || choice < 0 || choice > 2) {
      cin.clear();
      cin.ignore(10000, '\n');
      cout << "Invalid input. Please enter 0, 1, or 2.\n";
      continue;
    }
    return static_cast<UserMood>(choice);
  }
}
enum class GameSource { Steam, Local };

struct UnifiedGame {
    GameSource source = GameSource::Local;
    std::string name;
    long long igdb_id = 0;
    fs::path installDir;
    int appid = 0;
    fs::path gamePath;
};

static std::vector<fs::path> get_local_game_directories() {
    std::vector<fs::path> dirs;
    fs::path config_path = "local_game_dirs.txt";
    
    if (fs::exists(config_path)) {
        std::ifstream file(config_path);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '#') {
                dirs.push_back(line);
            }
        }
        if (!dirs.empty()) {
            return dirs;
        }
    }
    
    std::cout << "\n=== Local Games Configuration ===\n";
    std::cout << "No local game directories configured.\n";
    std::cout << "Please enter the full paths to your local game folders (e.g., E:\\Games).\n";
    std::cout << "Enter an empty line when you are finished.\n";
    
    // Clear any leftover newlines from previous formatted inputs
    if (std::cin.peek() == '\n') std::cin.ignore();

    std::string line;
    while (true) {
        std::cout << "Folder path (or press Enter to finish): ";
        std::getline(std::cin, line);
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty()) {
            if (dirs.empty()) {
                std::cout << "You must enter at least one directory.\n";
                continue;
            }
            break;
        }
        
        if (fs::exists(line) && fs::is_directory(line)) {
            dirs.push_back(line);
        } else {
            std::cout << "Invalid directory path or directory does not exist. Please try again.\n";
        }
    }
    
    std::ofstream out(config_path);
    if (out) {
        out << "# Local Game Directories\n";
        for (const auto& d : dirs) {
            out << d.string() << "\n";
        }
    }
    
    return dirs;
}

static std::vector<UnifiedGame> get_local_games() {
  std::vector<temp_GameEntry> localGames;
  auto gameDirs = get_local_game_directories();
  for (const auto& dir : gameDirs) {
      scan_directory_for_games(dir, localGames);
  }
  
  std::vector<UnifiedGame> games;
  games.reserve(localGames.size());
  for (const auto& g : localGames) {
      UnifiedGame ug;
      ug.source = GameSource::Local;
      ug.name = g.name;
      ug.igdb_id = g.igdb_id;
      ug.installDir = g.installDir;
      ug.gamePath = g.gamePath;
      games.push_back(std::move(ug));
  }
  return games;
}

static std::vector<UnifiedGame> get_steam_games() {
  std::vector<SteamGame> steamGames = read_installed_steam_games();
  std::vector<UnifiedGame> games;
  games.reserve(steamGames.size());
  for (const auto& g : steamGames) {
      UnifiedGame ug;
      ug.source = GameSource::Steam;
      ug.name = g.name;
      ug.igdb_id = g.igdb_id;
      ug.installDir = g.installDir;
      ug.appid = g.appid;
      games.push_back(std::move(ug));
  }
  return games;
}

static void run_games_menu(std::vector<UnifiedGame>& games, const std::string& menu_title) {
  if (games.empty()) {
    cout << "No games found for " << menu_title << ".\n";
    return;
  }

  std::sort(games.begin(), games.end(), [](const UnifiedGame &a, const UnifiedGame &b) {
      return to_lower(a.name) < to_lower(b.name);
  });

  while (true) {
    cout << "\n=== " << menu_title << " ===\n";
    for (size_t i = 0; i < games.size(); ++i) {
      cout << "  [" << (i + 1) << "] " << games[i].name;
      if (menu_title == "All Games") {
          if (games[i].source == GameSource::Steam) cout << " (Steam)";
          else cout << " (Local)";
      }

      double pref = get_game_preference(games[i].name);
      if (pref != 0.0) {
          cout << " [" << (pref > 0 ? "+" : "") << pref << "]";
      }

      std::string key = (games[i].igdb_id != 0)
                            ? "igdb_" + std::to_string(games[i].igdb_id)
                            : (games[i].source == GameSource::Steam ? "steam_" + std::to_string(games[i].appid) : "local_" + make_canonical(games[i].name));
      long long pt = get_playtime(key);
      if (pt > 0)
        cout << " (Playtime: " << (pt / 60) << " min played)";
      if (games[i].igdb_id != 0) {
        cout << " [IGDB: " << games[i].igdb_id << "]";
      } else {
        cout << " [Unrecognized]";
      }
      cout << "\n";
    }
    cout << "  [0] Back\n\n";

    cout << "Enter selection: ";
    int choice = -1;
    if (!(cin >> choice)) {
      cin.clear();
      cin.ignore(10000, '\n');
      cout << "Invalid input.\n";
      continue;
    }

    if (choice == 0)
      break;
    if (choice < 1 || choice > static_cast<int>(games.size())) {
      cout << "Invalid selection.\n";
      continue;
    }

    UnifiedGame &selected = games[choice - 1];

    while (true) {
      cout << "\n--- Game Details ---\n";
      cout << "Name: " << selected.name;
      if (selected.source == GameSource::Steam) cout << " (AppID: " << selected.appid << ")\n";
      else cout << "\n";

      std::string key = (selected.igdb_id != 0)
                            ? "igdb_" + std::to_string(selected.igdb_id)
                            : (selected.source == GameSource::Steam ? "steam_" + std::to_string(selected.appid) : "local_" + make_canonical(selected.name));
      long long pt = get_playtime(key);
      cout << "Total Playtime: ";
      if (pt > 0)
        cout << (pt / 60) << " minutes (" << pt << " seconds)\n";
      else
        cout << "Never played\n";
      if (selected.igdb_id != 0) {
        cout << "IGDB ID: " << selected.igdb_id << "\n";
        GameMetadata meta = get_game_metadata(selected.igdb_id);
        cout << "Developer: " << meta.developer << "\n";
        cout << "Rating: " << (meta.rating > 0 ? std::to_string(meta.rating) : "N/A") << "\n";
        if (meta.time_to_beat_seconds > 0) {
          cout << "Time to Beat: " << (meta.time_to_beat_seconds / 3600) << " hours\n";
        } else {
          cout << "Time to Beat: N/A\n";
        }
        
        cout << "Genres: ";
        if (!meta.all_genres.empty()) {
            for (size_t i = 0; i < meta.all_genres.size(); ++i) {
                cout << meta.all_genres[i] << (i + 1 < meta.all_genres.size() ? ", " : "");
            }
            cout << "\n";
        } else {
            cout << "Unknown\n";
        }
        
        cout << "[ML] Main Genres: ";
        if (!meta.main_genres.empty()) {
            for (size_t i = 0; i < meta.main_genres.size(); ++i) {
                cout << meta.main_genres[i] << (i + 1 < meta.main_genres.size() ? ", " : "");
            }
            cout << "\n";
        } else {
            cout << "Unknown\n";
        }
      } else {
        cout << "IGDB ID: Unrecognized\n";
      }

      cout << "Install Dir: " << selected.installDir.string() << "\n";
      if (selected.source == GameSource::Local) {
          cout << "Path: " << selected.gamePath.string() << "\n";
      }
      cout << "\n";

      cout << "  [1] Launch Game\n";
      cout << "  [2] Like\n";
      cout << "  [3] Dislike\n";
      cout << "  [4] Uninstall\n";
      cout << "  [0] Back to List\n\n";
      cout << "Enter selection: ";

      int action = -1;
      if (!(cin >> action)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input.\n";
        continue;
      }

      if (action == 0)
        break;
      
      if (action == 2) {
          double new_pref = toggle_game_preference(selected.name, 1.0);
          if (new_pref > 0) cout << "[✓] " << selected.name << " -> +1.0 (Liked)\n";
          else cout << "[✓] " << selected.name << " -> 0.0 (Neutral)\n";
          continue;
      } else if (action == 3) {
          double new_pref = toggle_game_preference(selected.name, -1.0);
          if (new_pref < 0) cout << "[✓] " << selected.name << " -> -1.0 (Disliked)\n";
          else cout << "[✓] " << selected.name << " -> 0.0 (Neutral)\n";
          continue;
      } else if (action == 4) {
          cout << "\n[?] Are you sure you want to uninstall \"" << selected.name << "\"? (y/N): ";
          std::string confirm;
          if (cin >> confirm && (confirm == "y" || confirm == "Y")) {
              if (selected.source == GameSource::Steam) {
                  if (uninstall_steam_game_by_appid(selected.appid)) {
                      cout << "[✓] Uninstall request sent for \"" << selected.name << "\" (AppID: " << selected.appid << ")\n";
                      cout << "    Steam will handle the rest.\n";
                  } else {
                      cout << "[WARN] Failed to send uninstall request to Steam.\n";
                  }
              } else {
                  std::error_code ec;
                  fs::remove_all(selected.installDir, ec);
                  if (!ec) {
                      cout << "[✓] Successfully uninstalled \"" << selected.name << "\".\n";
                      games.erase(games.begin() + choice - 1);
                      break; // Return to the main game list
                  } else {
                      cout << "[WARN] Failed to uninstall \"" << selected.name << "\": " << ec.message() << "\n";
                  }
              }
          } else {
              cout << "[✗] Uninstall cancelled.\n";
          }
          continue;
      } else if (action == 1) {
        cout << "\nLaunching: " << selected.name << "...\n";
        auto start_time = std::time(nullptr);
        
        if (selected.source == GameSource::Steam) {
            if (!launch_steam_game_by_appid(selected.appid)) {
              cout << "[WARN] Failed to launch via Steam protocol.\n";
              break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
            cout << "Monitoring process. Playtime is now recording...\n";
            while (is_game_running_in_dir(selected.installDir)) {
              std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            auto end_time = std::time(nullptr);
            if (end_time > start_time) {
              long long duration = end_time - start_time;
              record_play_session(key, selected.name, start_time, end_time);
              cout << "Game closed. Recorded " << duration << " seconds of playtime.\n";
            }
        } else {
            int code = launchGame(selected.gamePath);
            auto end_time = std::time(nullptr);
            
            if (code != 0) {
              cout << "[INFO] Process exited with code: " << code << "\n";
            }
            
            if (end_time > start_time) {
              long long duration = end_time - start_time;
              record_play_session(key, selected.name, start_time, end_time);
              cout << "Recorded " << duration << " seconds of playtime.\n";
            }
        }
        break; // Return to the main game list after game closes
      } else {
        cout << "Invalid selection.\n";
      }
    }
  }
}

static void run_all_mode() {
  auto localGames = get_local_games();
  auto steamGames = get_steam_games();
  
  std::vector<UnifiedGame> allGames;
  allGames.reserve(localGames.size() + steamGames.size());
  
  for (auto& g : localGames) allGames.push_back(std::move(g));
  for (auto& g : steamGames) allGames.push_back(std::move(g));
  
  run_games_menu(allGames, "All Games");
}

static void run_local_mode() {
  auto games = get_local_games();
  run_games_menu(games, "Local Games");
}

static void run_steam_mode() {
  auto games = get_steam_games();
  run_games_menu(games, "Steam Games");
}

int main() {
  UserMood mood = ask_user_mood();

  cout << "Checking for new games to download artwork...\n";
  auto localGamesInit = get_local_games();
  auto steamGamesInit = get_steam_games();
  std::vector<std::string> allGameNames;
  for (const auto& g : localGamesInit) allGameNames.push_back(g.name);
  for (const auto& g : steamGamesInit) allGameNames.push_back(g.name);
  ensure_steamgriddb_images(allGameNames);
  cout << "Artwork check complete.\n";

  while (true) {
    cout << "\n=== Vortex Game Launcher ===\n";
    cout << "  [1] All games\n";
    cout << "  [2] Steam games\n";
    cout << "  [3] Local directory games\n";
    cout << "  [0] Exit\n\n";
    cout << "Choose mode: ";

    int mode = -1;
    if (!(cin >> mode)) {
      cin.clear();
      cin.ignore(10000, '\n');
      cout << "Invalid input.\n";
      continue;
    }

    if (mode == 0) {
      cout << "Cya chap\n";
      break;
    } else if (mode == 1) {
      run_all_mode();
    } else if (mode == 2) {
      run_steam_mode();
    } else if (mode == 3) {
      run_local_mode();
    } else {
      cout << "Invalid selection.\n";
    }
  }

  return 0;
}