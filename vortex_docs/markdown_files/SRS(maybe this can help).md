# SOFTWARE REQUIREMENTS SPECIFICATION (SRS)
## Project: **Vortex – A Unified Intelligent Game Library/Launcher**
Version: 1.0  
Date: 2026-03-13  
Prepared by: Keerti Vardhan, Yuvraj Bhardwaj, Ayushmaan Kapruwan, Aditya Kediyal

---

## CONTENTS
1. INTRODUCTION  
1.1 Purpose of the Project  
1.2 Target of the Beneficiary  
1.3 Project Scope  
1.4 References  

2. PROJECT DESCRIPTION  
2.1 Reference Algorithm  
2.2 Data/Data Structures  
2.3 SWOT Analysis  
2.4 Project Features  
2.5 User Classes and Characteristics  
2.6 Design and Implementation Constraints  
2.7 Design Diagrams  
2.8 Assumptions and Dependencies  

3. SYSTEM REQUIREMENTS  
3.1 User Interface  
3.2 Software Interface  
3.3 Database Interface  
3.4 Protocols  

4. NON-FUNCTIONAL REQUIREMENTS  
4.1 Performance Requirements  
4.2 Security Requirements  
4.3 Software Quality Attributes  

5. OTHER REQUIREMENTS  

---

## 1. INTRODUCTION

Vortex is a desktop game library management application with an intelligent recommendation system.  
It unifies game discovery and launching from multiple sources (currently Steam + local directories) into one interface and tracks usage patterns for future personalized recommendations.

The system is being developed using C++ (core launcher services), with planned Qt/QML UI, SQLite for local storage, and a Python ML module for recommendation scoring using game metadata (including IGDB-derived features).

### 1.1 Purpose of the Project
The purpose of this project is to:
- Eliminate fragmented game access across multiple launchers.
- Provide a single launch point for user-owned games.
- Generate personalized recommendations based on game features, play behavior, and mood context.
- Build a lightweight modular architecture that can be extended progressively.

### 1.2 Target of the Beneficiary
Primary beneficiaries:
- **Casual Gamers**: Need quick and simple access to their installed games.
- **Enthusiast Gamers**: Need unified access across platforms and play behavior insights.
- **Student/Research Evaluators**: Need a practical demonstration of software + ML integration in a real-world domain.

### 1.3 Project Scope
#### In Scope (Current + Planned for this phase)
- Detect installed Steam games.
- Detect local games from user-defined directories by executable scanning.
- Launch games from unified list.
- Collect and store play/session related data (partially implemented; expanding).
- Integrate ML recommendation pipeline (in progress).

#### Out of Scope (Current academic phase)
- Full cloud synchronization.
- Multi-user collaborative filtering.
- Linux/macOS production support.
- Social/friends ecosystem.
- Real-time emotion detection via camera/biometrics.

### 1.4 References
1. M. Kozakov and N. Kozakova, *Development of a Recommendation System for Video Games*, 2025.  
2. Playnite Documentation: https://api.playnite.link/docs/manual/gettingStarted/gettingStartedOverview.html  
3. GOG Developer Docs: https://docs.gog.com/quick-start/  
4. Steamworks Documentation: https://partner.steamgames.com/doc/home  
5. IGDB API Docs: https://www.igdb.com/api  

---

## 2. PROJECT DESCRIPTION

### 2.1 Reference Algorithm
Vortex recommendation module follows a hybrid content-driven approach:

1. **Content Similarity**
   - TF-IDF vectorization over game metadata text (genres/themes/description).
   - Cosine similarity between candidate games and user-liked games.

2. **Preference Adjustment**
   - Like/Dislike feedback converted to weighted preference score.

3. **Mood Adjustment**
   - User-selected mood encoded using one-hot vector.
   - Mood-genre mapping modifies candidate score.

4. **Final Rank**
   - Weighted aggregation of similarity + preference + quality + exploration factor.

> Note: Current Python implementation is work-in-progress and currently produces some incorrect suggestions. Calibration and validation are active tasks.

### 2.2 Data/Data Structures
#### Core C++ Structures
- `temp_GameEntry`
  - `name: string`
  - `gamePath: filesystem::path`

- `SteamGame`
  - `appid: int`
  - `name: string`
  - `libraryPath: path`
  - `installDir: path`
  - `manifestPath: path`

#### Input Data Types
- Local executable data (`.exe` scanning)
- Steam manifest metadata (`appmanifest_*.acf`)
- External metadata from IGDB API (via Python integration)
- User interaction data: launch events, likes/dislikes, mood selections

### 2.3 SWOT Analysis
**Strengths**
- Unified launcher concept with practical utility.
- Native C++ performance.
- Extendable architecture for ML evolution.

**Weaknesses**
- Current recommendation quality is inconsistent.
- Partial integration between C++ and Python modules.
- UI not yet in final QML form.

**Opportunities**
- Add more platforms (GOG, Epic, etc.).
- Improve with stronger feature engineering and model tuning.
- Deploy as a lightweight alternative to heavy launchers.

**Threats**
- API changes/rate limits (Steam/IGDB).
- Data sparsity (cold start).
- Scope creep in academic timeline.

### 2.4 Project Features
- Multi-source game list (Steam + Local).
- Launch games directly from Vortex.
- Basic filtering of non-game executables while scanning directories.
- Sorting and selection via interactive menu.
- Planned: analytics dashboards, mood-based recommendation display.

### 2.5 User Classes and Characteristics
1. **End User (Gamer)**
   - Basic technical knowledge.
   - Needs simple interaction and fast response.

2. **Project Developer**
   - Works on modules (C++, Python, DB, UI).
   - Needs modular and testable components.

3. **Evaluator/Faculty**
   - Reviews methodology, architecture, and requirement traceability.

### 2.6 Design and Implementation Constraints
- Primary OS target: **Windows**.
- Steam data reading depends on Windows Registry + Steam file structure.
- Local scan currently assumes executable-based discovery (`.exe`).
- Recommendation quality depends on metadata completeness and user history.
- Time-bound implementation (12-week academic window).

### 2.7 Design Diagrams
Include the following diagrams in final document appendix:
- System Architecture Diagram (C++ Core ↔ SQLite ↔ Python ML ↔ UI)
- User Workflow Diagram
- Data Flow Diagram
- ML Pipeline Diagram
- PERT / Gantt diagrams from proposal

### 2.8 Assumptions and Dependencies
- Steam client is installed and accessible.
- User has locally installed game binaries.
- Internet access available when fetching IGDB metadata.
- Python runtime and required ML libraries are installed for recommendation module.
- Required permissions are available for file scanning and process launching.

---

## 3. SYSTEM REQUIREMENTS

### 3.1 User Interface
(Current prototype is CLI; final UI planned in Qt/QML)

#### Functional UI Requirements
- System shall display main menu with launcher modes.
- System shall display discovered games with index, title, and identifier/path.
- System shall allow user to select and launch game by input.
- System shall provide clear feedback for invalid input and launch failures.
- System shall display recommendation list with score in ranked order (when ML module invoked).

### 3.2 Software Interface
- **C++ Core Modules**
  - `game_manager`: local directory scan, sorting, launch.
  - `steam_manager`: Steam registry detection, library parsing, Steam launch.
- **Python Module**
  - Metadata ingestion from IGDB.
  - Feature extraction and score calculation.
- **Planned UI Layer**
  - Qt/QML frontend over C++ backend services.

### 3.3 Database Interface
- SQLite database (planned/partially integrated) shall store:
  - Game catalog records.
  - Session logs (start/end/playtime).
  - User preference flags (like/dislike).
  - Mood-session records.
  - Cached metadata references.

### 3.4 Protocols
- Local process execution (`std::system`, ShellExecute on Windows).
- Steam launch protocol URI: `steam://run/<appid>`.
- IGDB API access over HTTPS using authenticated requests (Twitch OAuth).
- Internal C++ ↔ Python exchange (file-based JSON or API bridge, depending on final implementation).

---

## 4. NON-FUNCTIONAL REQUIREMENTS

### 4.1 Performance Requirements
- Game list retrieval (Steam/local) should complete within acceptable desktop latency under standard library sizes.
- Launcher should avoid excessive memory usage during scan and ranking.
- Recommendation generation should complete in user-tolerable time for interactive usage.

### 4.2 Security Requirements
- No plaintext storage of external API secrets in source code.
- Inputs from external files/APIs must be validated before processing.
- File and process operations should be limited to intended game paths.
- Local user data should remain on-device unless explicitly exported.

### 4.3 Software Quality Attributes
- **Reliability**: Graceful handling of invalid paths, malformed manifests, missing registry keys.
- **Maintainability**: Modular components (`game_manager`, `steam_manager`, ML module).
- **Usability**: Simple, low-friction user flow.
- **Portability (Future)**: Current Windows-first design with future cross-platform consideration.
- **Scalability**: Support larger game libraries and added data features over time.

---

## 5. OTHER REQUIREMENTS

1. **Evaluation-Oriented Requirements**
   - Include requirement traceability mapping (Requirement → Module → Test Case).
   - Include evidence screenshots/logs for scanning, launching, and recommendation output.

2. **Testing Requirements**
   - Unit tests for parsing/scanning logic.
   - Integration tests for Steam detection and launch flow.
   - Validation tests for recommendation quality (precision@k / sanity checks).

3. **Documentation Requirements**
   - User manual (installation + usage).
   - Developer setup guide (build, dependencies, run steps).
   - Known limitations and future work section.

---

## Appendix A (Suggested)
- A1: Current implemented code summary (C++ modules)
- A2: Known issues (incorrect recommendations in current ML output)
- A3: Proposed corrective plan for recommendation engine:
  - feature normalization review
  - weighting rebalance
  - training/evaluation split
  - cold-start fallback refinement