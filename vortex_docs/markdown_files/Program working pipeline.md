Here’s the full pipeline, end-to-end.

### Program startup (`main.cpp`)
1. Program enters `main()`.
2. Shows top-level CLI menu:
   - `[1] Steam games`
   - `[2] Local directory games`
   - `[0] Exit`
3. Reads user input and routes to:
   - `run_steam_mode()` or
   - `run_local_mode()`.

---

### Local mode pipeline (`game_manager.*`)
`run_local_mode()`:
1. Defines scan roots:
   - `G:\Games`
   - `D:\Games`
2. Creates `vector<temp_GameEntry> games`.
3. Calls `scan_directory_for_games()` for each root.

`scan_directory_for_games()`:
1. Validates directory exists and is a folder.
2. Iterates each immediate subfolder (each treated as a game folder candidate).
3. Recursively walks files in that subfolder.
4. For each file:
   - must be regular file
   - must be `.exe` (`isLaunchableFile`)
   - exe name must **not** be “bad” (`isBadExeName`)  
     (blocks unitycrashhandler/setup/updater/unins/uninstall and names > 40 chars)
5. Among remaining valid exes in that folder, picks the **largest file**.
6. Saves one `temp_GameEntry { folderName, absoluteExePath }`.

Back in `run_local_mode()`:
1. Sorts collected games alphabetically (`sortGamesByName`).
2. Shows numbered list.
3. User chooses a game.
4. Calls `launchGame(path)`:
   - builds quoted command
   - executes with `std::system()`
5. If return code is `0`, logs current epoch time (`std::time(nullptr)`).
6. If non-zero, prints warning code.
7. Loops until user chooses Back (`0`).

---

### Steam mode pipeline (`steam_manager.*`)
`run_steam_mode()`:
1. Calls `read_installed_steam_games()`.

`read_installed_steam_games()`:
1. Gets Steam install path from registry:
   - `HKCU\Software\Valve\Steam\SteamPath`
2. Opens `<SteamPath>\steamapps\libraryfolders.vdf`.
3. Parses all `"path"` entries (Steam library locations).
4. For each library:
   - looks in `<library>\steamapps`
   - scans for `appmanifest_*.acf`
5. For each manifest:
   - extracts `appid`, `name`, `installdir` via regex
   - deduplicates by `appid` (`unordered_set<int>`)
   - builds `SteamGame` record:
     - appid
     - name
     - libraryPath
     - installDir (`library/steamapps/common/installdir`)
     - manifestPath
6. Sorts Steam games by name (then appid).
7. Returns `vector<SteamGame>`.

Back in `run_steam_mode()`:
1. Displays numbered Steam game list with AppIDs.
2. User selects game.
3. Launches via:
   - `launch_steam_game_by_appid(appid)`
   - internally `ShellExecuteA("steam://run/<appid>")`
4. If launch API fails, prints warning.
5. Loops until Back (`0`).

---

### Core data flow summary
- **Input**: user menu choice + game choice.
- **Discovery**:
  - Local: filesystem recursive exe scan + filtering + largest-exe heuristic.
  - Steam: registry → VDF libraries → ACF manifests.
- **Selection**: CLI numbered list.
- **Launch**:
  - Local: direct exe via `system`.
  - Steam: protocol URI via Steam client.
- **Feedback**: exit code/warnings and basic status prints.

---

### Key design choice
You have **two independent discovery+launch engines**:
- local heuristic launcher (`game_manager`)
- metadata-driven Steam launcher (`steam_manager`)

And `main.cpp` is just the orchestration layer/menu.