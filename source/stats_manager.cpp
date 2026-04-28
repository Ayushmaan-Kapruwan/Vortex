// ============================================================
//  stats_manager.cpp
//  Simple storage for game playtime statistics.
// ============================================================

#include "stats_manager.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

static const fs::path STATS_FILE_PATH = "playtime_stats.txt";

// Reads the playtime data from the file into an unordered_map
static std::unordered_map<long long, long long> load_stats() {
    std::unordered_map<long long, long long> stats;
    if (!fs::exists(STATS_FILE_PATH)) {
        return stats;
    }

    std::ifstream file(STATS_FILE_PATH);
    if (!file.is_open()) {
        std::cerr << "[WARN] Failed to open " << STATS_FILE_PATH << " for reading.\n";
        return stats;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos != std::string::npos) {
            try {
                long long id = std::stoll(line.substr(0, pos));
                long long time = std::stoll(line.substr(pos + 1));
                stats[id] = time;
            } catch (...) {
                // Ignore parsing errors for individual lines
            }
        }
    }
    return stats;
}

// Writes the playtime data from the unordered_map back to the file
static void save_stats(const std::unordered_map<long long, long long>& stats) {
    std::ofstream file(STATS_FILE_PATH, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "[WARN] Failed to open " << STATS_FILE_PATH << " for writing.\n";
        return;
    }

    file << "# Vortex Playtime Stats\n";
    file << "# Format: IGDB_ID=TOTAL_SECONDS\n";
    for (const auto& [id, time] : stats) {
        file << id << "=" << time << "\n";
    }
}

void add_playtime(long long igdb_id, long long duration_seconds) {
    if (igdb_id <= 0 || duration_seconds <= 0) return;

    auto stats = load_stats();
    stats[igdb_id] += duration_seconds;
    save_stats(stats);
}

long long get_playtime(long long igdb_id) {
    if (igdb_id <= 0) return 0;
    
    auto stats = load_stats();
    auto it = stats.find(igdb_id);
    if (it != stats.end()) {
        return it->second;
    }
    return 0;
}
