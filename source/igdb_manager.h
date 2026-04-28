#pragma once

#include <string>

// Queries the IGDB API and returns the canonical game name for the given
// folder/query string.  Falls back to the original folderName if the API
// returns no match, credentials are not set, or any network error occurs.
std::string igdb_resolve_name(const std::string& folderName);
