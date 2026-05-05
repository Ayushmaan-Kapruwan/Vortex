#pragma once

#include <string>
#include <vector>

// Ensures that SteamGridDB images (grid, logo, hero) exist for each game in the list.
// If an image folder/file already exists for a game, it skips downloading for that game.
void ensure_steamgriddb_images(const std::vector<std::string>& game_names);
