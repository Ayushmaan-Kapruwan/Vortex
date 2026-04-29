#include "preference_manager.h"
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <iostream>

namespace fs = std::filesystem;

static const fs::path PREF_FILE_PATH = "preferences.json";

// Very basic JSON parser/serializer since we don't have a library.
// Assumes format: {"Game Name": 1.0, ...}
static std::unordered_map<std::string, double> load_preferences() {
    std::unordered_map<std::string, double> prefs;
    if (!fs::exists(PREF_FILE_PATH)) return prefs;

    std::ifstream file(PREF_FILE_PATH);
    std::string line;
    while (std::getline(file, line)) {
        auto quote1 = line.find('"');
        if (quote1 == std::string::npos) continue;
        auto quote2 = line.find('"', quote1 + 1);
        if (quote2 == std::string::npos) continue;
        
        auto colon = line.find(':', quote2 + 1);
        if (colon == std::string::npos) continue;

        std::string name = line.substr(quote1 + 1, quote2 - quote1 - 1);
        std::string val_str = line.substr(colon + 1);
        
        // Remove trailing commas and spaces
        while (!val_str.empty() && (val_str.back() == ',' || val_str.back() == ' ' || val_str.back() == '\r')) {
            val_str.pop_back();
        }

        try {
            prefs[name] = std::stod(val_str);
        } catch (...) {}
    }
    return prefs;
}

static void save_preferences(const std::unordered_map<std::string, double>& prefs) {
    std::ofstream file(PREF_FILE_PATH, std::ios::trunc);
    if (!file.is_open()) return;

    file << "{\n";
    size_t count = 0;
    for (const auto& [name, score] : prefs) {
        if (score == 0.0) {
            count++;
            continue; // Skip saving neutral scores to keep file clean
        }
        
        // Escape quotes just in case
        std::string safe_name = name;
        size_t pos = 0;
        while ((pos = safe_name.find('"', pos)) != std::string::npos) {
            safe_name.replace(pos, 1, "\\\"");
            pos += 2;
        }

        file << "  \"" << safe_name << "\": " << score;
        if (count < prefs.size() - 1) {
            file << ",";
        }
        file << "\n";
        count++;
    }
    file << "}\n";
}

double get_game_preference(const std::string& game_name) {
    auto prefs = load_preferences();
    auto it = prefs.find(game_name);
    if (it != prefs.end()) {
        return it->second;
    }
    return 0.0;
}

double toggle_game_preference(const std::string& game_name, double target_score) {
    auto prefs = load_preferences();
    
    double current = 0.0;
    auto it = prefs.find(game_name);
    if (it != prefs.end()) {
        current = it->second;
    }

    double new_score = target_score;
    if (current == target_score) {
        new_score = 0.0; // Toggle off
    }

    prefs[game_name] = new_score;
    save_preferences(prefs);
    return new_score;
}
