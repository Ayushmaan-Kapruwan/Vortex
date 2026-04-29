#include "metadata_manager.h"
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

static const fs::path METADATA_FILE_PATH = "game_metadata.txt";

static std::unordered_map<long long, GameMetadata> load_metadata() {
    std::unordered_map<long long, GameMetadata> cache;
    if (!fs::exists(METADATA_FILE_PATH)) return cache;

    std::ifstream file(METADATA_FILE_PATH);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        try {
            long long id = std::stoll(line.substr(0, pos));
            std::string rest = line.substr(pos + 1);

            GameMetadata data;
            data.igdb_id = id;

            auto p1 = rest.find('|');
            if (p1 != std::string::npos) {
                data.developer = rest.substr(0, p1);
                auto p2 = rest.find('|', p1 + 1);
                if (p2 != std::string::npos) {
                    data.rating = std::stod(rest.substr(p1 + 1, p2 - p1 - 1));
                    data.time_to_beat_seconds = std::stoll(rest.substr(p2 + 1));
                }
            }
            cache[id] = data;
        } catch (...) {
        }
    }
    return cache;
}

static void save_metadata_cache(const std::unordered_map<long long, GameMetadata>& cache) {
    std::ofstream file(METADATA_FILE_PATH, std::ios::trunc);
    if (!file.is_open()) return;

    file << "# Vortex Game Metadata\n";
    file << "# Format: IGDB_ID=Developer|Rating|Time_To_Beat_Seconds\n";
    for (const auto& [id, data] : cache) {
        file << id << "=" << data.developer << "|" << data.rating << "|" << data.time_to_beat_seconds << "\n";
    }
}

void save_game_metadata(const GameMetadata& data) {
    if (data.igdb_id <= 0) return;
    auto cache = load_metadata();
    cache[data.igdb_id] = data;
    save_metadata_cache(cache);
}

GameMetadata get_game_metadata(long long igdb_id) {
    GameMetadata fallback;
    fallback.igdb_id = igdb_id;
    if (igdb_id <= 0) return fallback;

    auto cache = load_metadata();
    auto it = cache.find(igdb_id);
    if (it != cache.end()) {
        return it->second;
    }
    return fallback;
}
