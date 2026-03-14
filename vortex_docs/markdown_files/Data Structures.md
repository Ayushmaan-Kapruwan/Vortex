In your current codebase, these data structures are being used:

- `struct temp_GameEntry`
  - fields: `std::string name`, `fs::path gamePath`
  - used to represent one local game entry.

- `struct SteamGame` *(in steam logic file)*
  - fields like `appid`, `name`, `libraryPath`, `installDir`, `manifestPath`
  - used to represent one installed Steam game.

- `std::vector<temp_GameEntry>`
  - dynamic array of local game entries (`games` list).

- `std::vector<SteamGame>`
  - dynamic array of Steam game entries.

- `std::unordered_set<int>`
  - used in Steam scan to avoid duplicate AppIDs.

- `std::unordered_set<std::string>`
  - used in Steam library parsing to avoid duplicate library paths.

- `std::string`
  - text container throughout (names, commands, file contents).

- `std::filesystem::path`
  - path object for directories/files.

- `std::error_code`
  - lightweight error holder for filesystem operations (instead of throwing).

- `std::regex` + `std::smatch`
  - regex pattern and match result for parsing VDF/ACF text.

- `std::sregex_iterator`
  - iterator over all regex matches in a string.