#pragma once
#include <string>
#include <vector>

struct GameMetadata {
    long long igdb_id = 0;
    std::string developer = "Unknown";
    double rating = 0.0;
    long long time_to_beat_seconds = 0;
    std::vector<std::string> all_genres;
    std::vector<std::string> main_genres;
};

void save_game_metadata(const GameMetadata& data);
GameMetadata get_game_metadata(long long igdb_id);
