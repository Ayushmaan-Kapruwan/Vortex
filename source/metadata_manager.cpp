#include "metadata_manager.h"
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

static const fs::path METADATA_FILE_PATH = "game_metadata.txt";

static std::string to_lower_str(const std::string& str) {
    std::string out = str;
    for (char& c : out) c = std::tolower(c);
    return out;
}

static std::vector<std::string> split_str(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

static std::string join_str(const std::vector<std::string>& vec, char delimiter) {
    std::string res;
    for (size_t i = 0; i < vec.size(); ++i) {
        res += vec[i];
        if (i + 1 < vec.size()) res += delimiter;
    }
    return res;
}

static std::vector<std::string> derive_main_genres(const std::vector<std::string>& all_genres) {
    std::vector<std::string> priority = {"Shooter", "Adventure", "Simulator", "RPG", "Platform", "Puzzle", "Fighting", "Racing", "Visual Novel", "Indie"};
    std::vector<std::string> main_genres;

    for (const auto& genre : all_genres) {
        std::string lower_genre = to_lower_str(genre);
        bool is_priority = false;
        for (const auto& p : priority) {
            std::string lower_p = to_lower_str(p);
            if (lower_genre.find(lower_p) != std::string::npos) {
                is_priority = true;
                break;
            }
            if (p == "RPG" && lower_genre.find("role-playing") != std::string::npos) {
                is_priority = true;
                break;
            }
        }
        if (is_priority) {
            main_genres.push_back(genre);
            if (main_genres.size() == 2) break;
        }
    }

    if (main_genres.empty()) {
        for (size_t i = 0; i < std::min((size_t)2, all_genres.size()); ++i) {
            main_genres.push_back(all_genres[i]);
        }
    }

    return main_genres;
}

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
                    auto p3 = rest.find('|', p2 + 1);
                    if (p3 != std::string::npos) {
                        data.time_to_beat_seconds = std::stoll(rest.substr(p2 + 1, p3 - p2 - 1));
                        auto p4 = rest.find('|', p3 + 1);
                        if (p4 != std::string::npos) {
                            data.all_genres = split_str(rest.substr(p3 + 1, p4 - p3 - 1), ',');
                            data.main_genres = split_str(rest.substr(p4 + 1), ',');
                        } else {
                            data.all_genres = split_str(rest.substr(p3 + 1), ',');
                        }
                    } else {
                        data.time_to_beat_seconds = std::stoll(rest.substr(p2 + 1));
                    }
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
    file << "# Format: IGDB_ID=Developer|Rating|Time_To_Beat_Seconds|All_Genres|Main_Genres\n";
    for (const auto& [id, data] : cache) {
        file << id << "=" << data.developer << "|" << data.rating << "|" << data.time_to_beat_seconds 
             << "|" << join_str(data.all_genres, ',') << "|" << join_str(data.main_genres, ',') << "\n";
    }
}

void save_game_metadata(const GameMetadata& data) {
    if (data.igdb_id <= 0) return;
    auto cache = load_metadata();
    
    GameMetadata copy = data;
    if (copy.main_genres.empty() && !copy.all_genres.empty()) {
        copy.main_genres = derive_main_genres(copy.all_genres);
    }
    
    cache[copy.igdb_id] = copy;
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
