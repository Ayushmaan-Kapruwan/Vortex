#include <ctime>
#include <iostream>
#include <vector>

#include "game_manager.h"
#include "igdb_manager.h"
#include "stats_manager.h"
#include "steam_manager.h"

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

static void run_local_mode() {
  fs::path gameDir1 = "E:\\Games";
  fs::path gameDir2 = "D:\\Games";

  std::vector<temp_GameEntry> games;
  games.reserve(256);

  scan_directory_for_games(gameDir1, games);
  scan_directory_for_games(gameDir2, games);

  if (games.empty()) {
    cout << "No local games found.\n";
    cout << "  - " << gameDir1 << "\n";
    cout << "  - " << gameDir2 << "\n";
    return;
  }

  sortGamesByName(games);

  while (true) {
    cout << "\n=== Local Games ===\n";
    for (size_t i = 0; i < games.size(); ++i) {
      cout << "  [" << (i + 1) << "] " << games[i].name;
      std::string key = (games[i].igdb_id != 0)
                            ? "igdb_" + std::to_string(games[i].igdb_id)
                            : "local_" + make_canonical(games[i].name);
      long long pt = get_playtime(key);
      if (pt > 0)
        cout << " (Playtime: " << (pt / 60) << " min played)";
      if (games[i].igdb_id != 0) {
        cout << " [IGDB: " << games[i].igdb_id << "]";
      } else {
        cout << " [Unrecognized]";
      }
      cout << "  (" << games[i].gamePath.string() << ")\n";
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

    const temp_GameEntry &selected = games[choice - 1];

    while (true) {
      cout << "\n--- Game Details ---\n";
      cout << "Name: " << selected.name << "\n";

      std::string key = (selected.igdb_id != 0)
                            ? "igdb_" + std::to_string(selected.igdb_id)
                            : "local_" + make_canonical(selected.name);
      long long pt = get_playtime(key);
      cout << "Total Playtime: ";
      if (pt > 0)
        cout << (pt / 60) << " minutes (" << pt << " seconds)\n";
      else
        cout << "Never played\n";
      if (selected.igdb_id != 0)
        cout << "IGDB ID: " << selected.igdb_id << "\n";
      else
        cout << "IGDB ID: Unrecognized\n";

      cout << "Path: " << selected.gamePath.string() << "\n\n";

      cout << "  [1] Launch Game\n";
      cout << "  [0] Back to List\n\n";
      cout << "Enter selection: ";

      int sub_choice = -1;
      if (!(cin >> sub_choice)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input.\n";
        continue;
      }

      if (sub_choice == 0) {
        break;
      } else if (sub_choice == 1) {
        cout << "\nLaunching: " << selected.name << "...\n";

        auto start_time = std::time(nullptr);
        int code = launchGame(selected.gamePath);
        auto end_time = std::time(nullptr);

        if (code == 0 || code == -1) {
          if (end_time > start_time) {
            long long duration = end_time - start_time;
            std::string key = (selected.igdb_id != 0)
                                  ? "igdb_" + std::to_string(selected.igdb_id)
                                  : "local_" + make_canonical(selected.name);
            record_play_session(key, selected.name, start_time, end_time);
            cout << "Recorded " << duration << " seconds of playtime.\n";
          }
        } else {
          cout << "[WARN] Process returned code: " << code << "\n";
        }
        break; // Return to the main game list after game closes
      } else {
        cout << "Invalid selection.\n";
      }
    }
  }
}

static void run_steam_mode() {
  std::vector<SteamGame> games = read_installed_steam_games();

  if (games.empty()) {
    cout << "No installed Steam games found.\n";
    return;
  }

  while (true) {
    cout << "\n=== Steam Games ===\n";
    for (size_t i = 0; i < games.size(); ++i) {
      cout << "  [" << (i + 1) << "] " << games[i].name
           << " (AppID: " << games[i].appid << ")";
      std::string key = (games[i].igdb_id != 0)
                            ? "igdb_" + std::to_string(games[i].igdb_id)
                            : "steam_" + std::to_string(games[i].appid);
      long long pt = get_playtime(key);
      if (pt > 0)
        cout << " [Playtime: " << (pt / 60) << " min played]";
      if (games[i].igdb_id != 0)
        cout << " [IGDB: " << games[i].igdb_id << "]";
      else
        cout << " [Unrecognized]";
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

    SteamGame &selected = games[choice - 1];

    while (true) {
      cout << "\n--- Steam Game Details ---\n";
      cout << "Name: " << selected.name << " (AppID: " << selected.appid
           << ")\n";

      std::string key = (selected.igdb_id != 0)
                            ? "igdb_" + std::to_string(selected.igdb_id)
                            : "steam_" + std::to_string(selected.appid);
      long long pt = get_playtime(key);
      cout << "Total Playtime: ";
      if (pt > 0)
        cout << (pt / 60) << " minutes (" << pt << " seconds)\n";
      else
        cout << "Never played\n";
      if (selected.igdb_id != 0)
        cout << "IGDB ID: " << selected.igdb_id << "\n";
      else
        cout << "IGDB ID: Unrecognized\n";

      cout << "Install Dir: " << selected.installDir.string() << "\n\n";

      cout << "  [1] Launch Game\n";
      cout << "  [0] Back to List\n\n";
      cout << "Enter selection: ";

      int sub_choice = -1;
      if (!(cin >> sub_choice)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input.\n";
        continue;
      }

      if (sub_choice == 0) {
        break;
      } else if (sub_choice == 1) {
        cout << "\nLaunching (Steam): " << selected.name << "...\n";

        auto start_time = std::time(nullptr);
        if (!launch_steam_game_by_appid(selected.appid)) {
          cout << "[WARN] Failed to launch via Steam protocol.\n";
          break;
        }

        // Wait a few seconds for Steam to process the request and spawn the
        // game
        std::this_thread::sleep_for(std::chrono::seconds(5));

        cout << "Monitoring process. Playtime is now recording...\n";
        // Polling loop: checks every 2 seconds if any process in installDir is
        // running
        while (is_game_running_in_dir(selected.installDir)) {
          std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        auto end_time = std::time(nullptr);

        if (end_time > start_time) {
          long long duration = end_time - start_time;
          std::string key = (selected.igdb_id != 0)
                                ? "igdb_" + std::to_string(selected.igdb_id)
                                : "steam_" + std::to_string(selected.appid);
          record_play_session(key, selected.name, start_time, end_time);
          cout << "Game closed. Recorded " << duration
               << " seconds of playtime.\n";
        }
        break; // Return to the main game list
      } else {
        cout << "Invalid selection.\n";
      }
    }
  }
}

int main() {
  UserMood mood = ask_user_mood();

  while (true) {
    cout << "\n=== Vortex Game Launcher ===\n";
    cout << "  [1] Steam games\n";
    cout << "  [2] Local directory games\n";
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
      run_steam_mode();
    } else if (mode == 2) {
      run_local_mode();
    } else {
      cout << "Invalid selection.\n";
    }
  }

  return 0;
}