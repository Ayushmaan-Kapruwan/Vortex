#pragma once

#include <string>

// Gets the preference score for a game (e.g. 1.0 for liked, -1.0 for disliked, 0.0 for neutral).
double get_game_preference(const std::string& game_name);

// Toggles the preference score for a game. If the current score matches target_score, it resets to 0.0.
// Otherwise, it sets the score to target_score. Returns the new score.
double toggle_game_preference(const std::string& game_name, double target_score);
