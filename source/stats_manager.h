#pragma once

// Adds duration_seconds to the total playtime for the given igdb_id.
void add_playtime(long long igdb_id, long long duration_seconds);

// Retrieves the total playtime in seconds for the given igdb_id.
// Returns 0 if no playtime has been recorded.
long long get_playtime(long long igdb_id);
