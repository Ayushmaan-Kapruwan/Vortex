#pragma once

#include <string>

struct IgdbGameInfo {
    std::string name;
    long long id = 0;
    std::string developer = "Unknown";
    double rating = 0.0;
    long long time_to_beat_seconds = 0;
};

// Queries the IGDB API and returns the canonical game name and ID for the given
// folder/query string.  Falls back to the original folderName (and id 0) if the
// API returns no match, credentials are not set, or any network error occurs.
IgdbGameInfo igdb_resolve_game(const std::string &folderName,
                               bool interactive = true);
