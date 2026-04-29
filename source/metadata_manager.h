#pragma once
#include <string>

struct GameMetadata {
    long long igdb_id = 0;
    std::string developer = "Unknown";
    double rating = 0.0;
    long long time_to_beat_seconds = 0;
};

void save_game_metadata(const GameMetadata& data);
GameMetadata get_game_metadata(long long igdb_id);
