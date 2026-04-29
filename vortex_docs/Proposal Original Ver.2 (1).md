
## Project Title: Vortex: A Unified Intelligent Game Library/Launcher

Domain: Intelligent Systems & Software Engineering

Proposed By: 
- Keerti Vardhan
- Yuvraj Bhardwaj
- Ayushmaan Kapruwan
- Aditya Kediyal

---
### Terminologies Used

1. Platforms - Game distribution platforms, where a user can buy or subscribe to games. ^9831eb
2. Launcher - An interface/application that launches or opens a game executable upon user input. ^483314
3. TF-IDF Vectorize - TF-IDF Vectorizer is a technique used in Natural Language Processing (NLP) to convert text documents into numerical vectors that machine-learning models can understand. <sup> [[#^c03d51|1]] </sup>
   TF-IDF stands for Term Frequency – Inverse Document Frequency.
4. One-Hot Encoding - One-Hot Encoding is a technique used in Machine Learning and NLP to convert categorical data (labels or words) into a numerical binary format that models can process. <sup> [[#^c03d51|1]] </sup>
5. IGDB - IGDB stands for Internet Game Database. It is a large structured database of video game information. The IGDB API is the official interface that lets developers programmatically fetch this data for use in their applications/projects. <sup> [[#^7e8bd0|5]] </sup>
### 1. Abstract

Vortex is a unified game library management application with an intelligent recommendation system. The app detects games from multiple platforms into a single interface & also tracks playtime, session patterns, and user preferences. 

Built using C++ and Qt/QML for performance, Vortex implements a machine learning-based recommendation engine that uses content-based filtering and mood-aware personalization. The system analyses user behavior like play duration, time-of-day patterns, and explicit ratings to generate relevant game suggestions.  
A modular architecture allows users to customize features according to their needs, avoiding the bloat common in existing launchers. The project demonstrates practical application of recommender systems, data pipeline design, and native software development.  

Keywords: Game Library Management, Recommender Systems, Mood-Aware, Personalization, Desktop Application, Machine Learning

---

### 2. Introduction

#### Background

Digital game distribution has evolved significantly over the past decade. Users now own games across Steam, Epic Games Store, GOG, Xbox Game Pass, and other [[#^9831eb|platforms]]. Each platform maintains its own [[#^483314|launcher]], library interface, and usage statistics, creating a fragmented experience for gamers who must switch between multiple applications.

#### Problem Statement

Current game launchers operate in isolation, offering no unified view of a user's complete game collection. Playtime data remains isolated, making it impossible to analyze overall gaming habits. Existing recommendation systems rely primarily on purchase history and popularity metrics rather than actual user behavior and contextual factors like mood or available time.

#### Motivation

Our users deserve a single application that:
- Aggregates all their games regardless of platform
- Provides meaningful insights into their gaming habits
	- Recommends games based on current mood and context, not just past purchases

---

### 3. Literature Review

#### Existing Solutions

| Solution       | Description                        | Limitations                                                                    |
| -------------- | ---------------------------------- | ------------------------------------------------------------------------------ |
| Playnite       | Open-source unified game library   | No ML-based recommendations, basic UI <sup>[ [[#^de10d3\|2]] ]</sup>           |
| GOG Galaxy 2.0 | Multi-platform library aggregation | Limited personalization, no mood-aware features <sup>[ [[#^6db295\|3]] ]</sup> |
| Steam          | Largest PC game platform           | Single platform only, generic recommendations <sup>[ [[#^9a6fda\|4]] ]</sup>   |

#### Academic Background

Content-based filtering and collaborative filtering are established techniques in recommender systems. Netflix, Spotify, and YouTube employ hybrid approaches combining multiple models. Context-aware recommendation systems that consider time, location, and user state have shown improved user satisfaction in research studies. <sup>[ [[#^c03d51|1]] ]</sup>

A research paper i.e. “Development Of A Recommendation System For Video Games” has been studied primarily for the understanding of this entire project. The paper talks about the need for an hybrid recommendation engine for games and how to achieve this via various quantification and ML integration techniques. <sup>[ [[#^c03d51|1]] ]</sup>

#### Gap Analysis

| Gap                    | Description                                                     |
| ---------------------- | --------------------------------------------------------------- |
| No Unified Analytics   | Existing launchers don't combine playtime data across platforms |
| Static Recommendations | Suggestions don't adapt to time-of-day or session patterns      |
| Resource Heavy         | Most launchers consume significant system resources             |
| No Mood Inclusion      | No system considers user's current gaming mood                  |

---

### 4. Objectives and Scope

#### Primary Objectives

1. Unified Library Management — Aggregate games from multiple sources into a single, searchable interface

2. Intelligent Playtime Tracking — Monitor session duration, time-of-day patterns, and engagement metrics

3. ML-Powered Recommendations — Implement content-based filtering with mood-aware adjustments

4. Lightweight Modular Design — Create a performant application where features can be toggled on/off

#### Scope

| In Scope                                | Out of Scope                                       |
| --------------------------------------- | -------------------------------------------------- |
| Auto-detection from supported platforms |                                                    |
| Local playtime tracking                 | Cloud sync across devices                          |
| Content-based recommendations           | Collaborative filtering (requires multi-user data) |
| Mood input upon each application launch | Advanced emotion detection                         |
| Windows support                         | Mac/Linux support (future work)                    |
| Like/Dislike preferences                | Social features, friend lists                      |

*Supported platforms: Steam & GOG(for now).*

---

### 5. Methodology

#### Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        VORTEX ARCHITECTURE                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌─────────────┐     ┌─────────────┐     ┌─────────────┐       │
│   │   QML UI    │◄───►│   C++ Core  │◄───►│   SQLite    │       │
│   │  (Frontend) │     │  (Backend)  │     │    (Data)   │       │
│   └─────────────┘     └──────┬──────┘     └──────┬──────┘       │
│                              │                   │              │
│                              ▼                   ▼              │
│                       ┌─────────────────────────────┐           │
│                       │     Python ML Module        │           │
│                       │  (Recommendations Engine)   │           │
│                       └─────────────────────────────┘           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### Technology Stack

| Category | Technology | Justification |
|----------|------------|---------------|
| Core Language | C++17 | Performance, system access |
| UI Framework | Qt 6 / QML | Native performance, cross-platform potential |
| Database | SQLite | Lightweight, local storage, no server needed |
| ML Language | Python 3.10+ | Rich ML ecosystem, rapid prototyping |
| ML Libraries | scikit-learn, pandas, numpy | Industry standard, well-documented |
| Build System | CMake | Cross-platform build management |

#### ML Approach

| Model                | Purpose                                 | Technique                                                               |
| -------------------- | --------------------------------------- | ----------------------------------------------------------------------- |
| Content-Based Filter | Game similarity                         | TF-IDF vectorization + Cosine similarity on genres/themes               |
| Mood Adjustment      | User selects current mood on app launch | One-hot encoding + Mood-genre weight mapping                            |
| Hybrid Ranker        | Final recommendations                   | Weighted combination of content score + mood adjustment + quality score |

##### Mood Selection Options

[ ] 😌 **Relaxed**
[ ] 🎯 **Competitive**
[ ] 📖 Immersive

#### ML Pipeline

```
┌─────────────────────────────────────────────────────────────────┐
│                    ML PIPELINE                                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   USER INPUT                                                    │
│   ──────────                                                    │
│   • Liked/Disliked games                                        │
│   • Mood selection (😌 🎯 📖)                                  │
│                                                                 │
│         │                           │                           │
│         ▼                           ▼                           │
│   ┌───────────────┐          ┌───────────────┐                  │
│   │ CONTENT-BASED │          │    MOOD       │                  │
│   │    FILTER     │          │  ADJUSTMENT   │                  │
│   │               │          │               │                  │
│   │ TF-IDF +      │          │ One-hot +     │                  │
│   │ Cosine Sim    │          │ Genre weights │                  │
│   └───────┬───────┘          └───────┬───────┘                  │
│           │                          │                          │
│           │      ┌───────────────┐   │                          │
│           └─────►│ HYBRID RANKER │◄──┘                          │
│                  │               │                              │
│                  │ Weighted      │                              │
│                  │ Combination   │                              │
│                  └───────┬───────┘                              │
│                          │                                      │
│                          ▼                                      │
│                  ┌───────────────┐                              │
│                  │    RANKED     │                              │
│                  │RECOMMENDATIONS│                              │
│                  └───────────────┘                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### Feature Engineering Pipeline

```
Raw Data → Session Logs → Feature Extraction → User Vectors → ML Models → Recommendations
              │
              ├── Playtime (total, average, variance)
              ├── Session patterns (length, frequency, idle time)
              ├── Temporal signals (hour, weekday/weekend)
              └── Explicit preferences (likes, dislikes)
              └── Mood Session ID
```

#### Cold Start Problem

If the user is new to the launcher with no playtime hours and no preferred genre in the database. The question arises-

"How does the system recommend games when I first install it and have 0 playtime recorded?"

Solution -

In the initial stage we just ask the user on first setup of the launcher application, what genre does he prefer and we can evaluate a recommendation based off of that data.

---

### 6. System Design

#### System Workflow

```
┌────────────────────────────────────────────────────────────────────────┐
│                       USER WORKFLOW                                    │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌─────────────────┐   │
│  │  Add     │───►│  Play    │───►│  Track   │───►│  Get            │   │
│  │  Games   │    │  Game    │    │  Session │    │  Recommendations│   │
│  └──────────┘    └──────────┘    └──────────┘    └─────────────────┘   │
│       │              │               │               │                 │
│       ▼              ▼               ▼               ▼                 │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐          │
│  │ Library  │    │ Launcher │    │  Data    │    │   ML     │          │
│  │ Manager  │    │ Service  │    │ Pipeline │    │  Engine  │          │
│  └──────────┘    └──────────┘    └──────────┘    └──────────┘          │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘
```

#### Data Flow

| Stage      | Input           | Process                         | Output                 |
| ---------- | --------------- | ------------------------------- | ---------------------- |
| Collection | User actions    | Event capture                   | Raw events             |
| Storage    | Raw events      | Validation, storage             | SQLite records         |
| Processing | SQLite records  | Aggregation, feature extraction | Feature vectors        |
| Inference  | Feature vectors | ML model prediction             | Ranked recommendations |
| Display    | Recommendations | UI rendering                    | User sees suggestions  |

##### Data Usage and Utilization


| **Category**       | **Data Item**           | **Used in Launcher** | **Used for ML Recommendation** |
| ------------------ | ----------------------- | -------------------- | ------------------------------ |
| Local Data         | Game Name               | Yes                  | Yes                            |
| Local Data         | Version                 | Yes                  |                                |
| Local Data         | Install Directory       | Yes                  |                                |
| IGDB (External DB) | Genre/Tags              | Yes                  | Yes                            |
| IGDB (External DB) | Images                  | Yes                  |                                |
| IGDB (External DB) | Game Length             | Yes                  | Yes                            |
| IGDB (External DB) | Developer/Publisher     | Yes                  |                                |
| IGDB (External DB) | Ratings/Reviews         | Yes                  |                                |
| Produced Data      | Session Start/End       | Yes                  | Yes                            |
| Produced Data      | Idle/ Inactive duration |                      | Yes                            |
| Produced Data      | Session length          |                      | Yes                            |
| Produced Data      | Mood Session ID         | Yes                  | Yes                            |
| Produced Data      | Liked/Disliked Games    | Yes                  | Yes                            |
| Distribution API   | In-game Achievements    | Yes                  |                                |

##### Initial Game Genres Being Utilized

1. Shooter
2. Adventure
3. Simulator
4. RPG
5. Platform
6. Puzzle
7. Fighting
8. Racing
9. Visual Novel
10. Indie

| Mood           | Genres                                            |
| -------------- | ------------------------------------------------- |
| 😌 Relaxed     | Simulator, Puzzle, Adventure, Visual Novel, Indie |
| 🎯 Competitive | Shooter, Fighting, Racing                         |
| 📖 Immersive   | Role-playing (RPG), Adventure, Visual Novel       |

##### IGDB API (Metadata source) <sup> [[#^c03d51|1]] </sup>

```
┌─────────────────────────────────────────────────────────────────┐
│                      IGDB API SUMMARY                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   Format:         JSON                                          │
│   Access:         REST API (HTTP POST requests)                 │
│   Download:       No database dump available                    │
│   Authentication: Twitch OAuth required                         │
│   Cost:           Free                                          │
│   Rate Limit:     4 requests/second                             │
│                                                                 │
│   Base URL:       https://api.igdb.com/v4                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

###### IGDB API Flow

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                         IGDB DATA FLOW                                          │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│   ┌─────────────┐      HTTP POST       ┌─────────────┐                          │
│   │   VORTEX    │  ─────────────────►  │  IGDB API   │                          │
│   │   (Client)  │                      │  (Server)   │                          │
│   └─────────────┘  ◄─────────────────  └─────────────┘                          │
│                       JSON Response                                             │
│                                                                                 │
│   NOT a download — Real-time API calls                                          │
│                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────┘
```

##### Mood Based Results(minimal)

```
┌─────────────────────────────────────────────────────────────────┐
│                    MOOD → GAME TYPE                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   😌 RELAXED                                                    │
│   └── Best: Adenture, Simulator                                 │
│   └── Avoid: Horror, Intense shooters                           │
│                                                                 │
│   🎯 COMPETITIVE                                                │
│   └── Best: Shooters                                            │
│   └── Avoid: RPGs, Adventure                                    │
│                                                                 │
│   📖 IMMERSIVE                                                  │
│   └── Best: RPGs, Story-rich, Open world, Adventure             │
│   └── Avoid: Quick arcade games, Sports                         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

##### Data Quantification

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                      DATA TO ML QUANTIFICATION PIPELINE                         │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│   RAW DATA                 PROCESSING              QUANTIFIED OUTPUT            │
│   ────────                 ──────────              ─────────────────            │
│                                                                                 │
│   "Action, RPG"    ──►    One-Hot Encoding   ──►   [1,0,0,1,0,0,0,0]            │
│                                                                                 │
│   "Fantasy game    ──►    TF-IDF Vectorize   ──►   [0.23, 0.45, 0.12...]        │
│    with magic"                                                                  │
│                                                                                 │
│   Like/Dislike     ──►    Preference Score   ──►   +1.0 / -1.0                  │
│                                                                                 │
│   120 minutes      ──►    Normalization      ──►   0.75 (scaled 0-1)            │
│                                                                                 │
│   "Relaxed" mood   ──►    Mood Vector        ──►   [1,0,0,0]                    │
│                                                                                 │
│   22:00 hour       ──►    Time Encoding      ──►   [0.92] or [0,0,0,1]          │
│                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────┘
```

###### Genre One-Hot Encoding

```
┌─────────────────────────────────────────────────────────────────┐
│                  GENRE ONE-HOT ENCODING                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Game: "Elden Ring"                                             │
│  Genres: ["Action", "RPG", "Adventure"]                         │
│                                                                 │
│  Genre Vector:                                                  │
│  ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐  │
│  │ Act │ Adv │ RPG │ Str │ Sim │ Spo │ Rac │ Ind │ Cas │ ... │  │
│  ├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤  │
│  │  1  │  1  │  1  │  0  │  0  │  0  │  0  │  0  │  0  │  0  │  │
│  └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘  │
│                                                                 │
│  Game: "Stardew Valley"                                         │
│  Genres: ["Simulation", "Indie", "Casual", "RPG"]               │
│                                                                 │
│  Genre Vector:                                                  │
│  ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐  │
│  │ Act │ Adv │ RPG │ Str │ Sim │ Spo │ Rac │ Ind │ Cas │ ... │  │
│  ├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤  │
│  │  0  │  0  │  1  │  0  │  1  │  0  │  0  │  1  │  1  │  0  │  │
│  └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

###### Playtime & Engagement Metrics

```
┌─────────────────────────────────────────────────────────────────┐
│                  SESSION DATA QUANTIFICATION                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Game: "Elden Ring"                                             │
│  Total Playtime: 125 hours                                      │
│  Sessions: 45                                                   │
│  Avg Session: 2.8 hours                                         │
│  Game Length: 100 hours                                         │
│                                                                 │
│  Engagement Score Calculation:                                  │
│  ├── Playtime Score:  125/100 = 1.0 (capped)                    │
│  ├── Frequency Score: 45/50 = 0.9                               │
│  ├── Length Score:    168/120 = 1.0 (capped)                    │
│  └── Final: 0.5(1.0) + 0.3(0.9) + 0.2(1.0) = 0.97               │
│                                                                 │
│  Temporal Features:                                             │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Morning (6-12):    5%   ██░░░░░░░░░░░░░░░░░░░░░░░░░░   │    │
│  │  Afternoon (12-18): 15%  ████░░░░░░░░░░░░░░░░░░░░░░░░   │    │
│  │  Evening (18-22):   45%  ████████████░░░░░░░░░░░░░░░░   │    │
│  │  Night (22-6):      35%  ██████████░░░░░░░░░░░░░░░░░░   │    │
│  │  Weekend:           60%  ████████████████░░░░░░░░░░░░   │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│  Session Vector: [0.97, 0.05, 0.15, 0.45, 0.35, 0.60]           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Final Session Vector:**
```
┌─────────────────────────────────────────────────────────────────┐
│                    SESSION VECTOR                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   Position:  [  1  ,   2   ,    3     ,    4    ,   5  ,   6  ] │
│   Meaning:   [Engage, Morn , Afternoon, Evening , Night, Wkend] │
│   Values:    [0.97 , 0.05 ,   0.15   ,  0.45   , 0.35 , 0.60 ]  │
│                                                                 │
│   This vector tells ML:                                         │
│   • Very engaged with this game (0.97)                          │
│   • Rarely plays mornings (0.05)                                │
│   • Mainly plays evenings (0.45) and nights (0.35)              │
│   • Prefers weekends (0.60)                                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

###### Mood Quantification

```
┌─────────────────────────────────────────────────────────────────┐
│                    MOOD QUANTIFICATION                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Selected Mood: "relaxed"                                       │
│                                                                 │
│  Mood One-Hot Vector:                                           │
│  ┌──────────┬─────────────┬───────────┬────────────┐            │
│  │ Relaxed  │ Competitive │ Immersive │ Quick Play │            │
│  ├──────────┼─────────────┼───────────┼────────────┤            │
│  │    1     │      0      │     0     │     0      │            │
│  └──────────┴─────────────┴───────────┴────────────┘            │
│                                                                 │
│  Mood-Genre Weights for "Relaxed":                              │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Simulation: +1.0  ████████████████████░░░░░░░░░░░░░░   │    │
│  │  Casual:     +1.0  ████████████████████░░░░░░░░░░░░░░   │    │
│  │  Puzzle:     +0.8  ████████████████░░░░░░░░░░░░░░░░░░   │    │
│  │  Indie:      +0.7  ██████████████░░░░░░░░░░░░░░░░░░░░   │    │
│  │  Horror:     -0.8  ░░░░░░░░░░░░░░░░░░░░████████████████ │    │
│  │  Shooter:    -0.5  ░░░░░░░░░░░░░░░░░░░░░░░░██████████   │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│  Game: "Stardew Valley" (Simulation, Indie, Casual)             │
│  Mood Match Score: (1.0 + 0.7 + 1.0) / 3 = +0.90                │
│                                                                 │
│  Game: "Dark Souls" (Action, RPG)                               │
│  Mood Match Score: (0.0 + 0.0) / 2 = 0.00                       │
│                                                                 │
│  Game: "Resident Evil" (Horror, Action)                         │
│  Mood Match Score: (-0.8 + 0.0) / 2 = -0.40                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**How it helps in recommendation**

```
┌─────────────────────────────────────────────────────────────────┐
│              SESSION DATA → BETTER RECOMMENDATIONS              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   WHAT SESSION DATA TELLS US:                                   │
│                                                                 │
│   High Evening + Night ratios                                   │
│   └── User prefers immersive games (has time to focus)          │
│                                                                 │
│   High Morning ratio                                            │
│   └── User might prefer short casual games                      │
│                                                                 │
│   High Weekend ratio                                            │
│   └── User has more time on weekends (suggest longer games)     │
│                                                                 │
│   High Engagement on RPGs                                       │
│   └── Recommend more RPGs                                       │
│                                                                 │
│   Low Engagement on Shooter games                               │
│   └── Don't recommend shooter games                             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

###### Data to Score

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                    COMPLETE QUANTIFICATION SUMMARY                              │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  INPUT DATA              QUANTIFICATION           OUTPUT DIMENSION              │
│  ──────────              ──────────────           ────────────────              │
│                                                                                 │
│  Genres                  One-Hot Encoding         15 dimensions                 │
│  ["Action", "RPG"]  ──►  [1,0,1,0,0,0...]    ──►  (binary)                      │
│                                                                                 │
│  Description             TF-IDF Vectorize         100 dimensions                │
│  "Fantasy RPG..."   ──►  [0.23, 0.45...]     ──►  (float 0-1)                   │
│                                                                                 │
│  Metacritic              Normalization            1 dimension                   │
│  94                 ──►  0.94                ──►  (float 0-1)                   │
│                                                                                 │
│  Like/Dislike            Preference Score         15 dimensions                 │
│  {likes, dislikes}  ──►  [0.8, -0.3, 0.5...]──►  (float -1 to 1)                │
│                                                                                 │
│  Session Data            Temporal Features        6 dimensions                  │
│  {time, duration}   ──►  [0.97, 0.05, 0.45] ──►  (float 0-1)                    │
│                                                                                 │
│  Mood                    One-Hot + Weights        4 dimensions                  │
│  "relaxed"          ──►  [1, 0, 0, 0]        ──►  (binary)                      │
│                                                                                 │
│  ─────────────────────────────────────────────────────────────────────────────  │
│                                                                                 │
│  FINAL RECOMMENDATION SCORE:                                                    │
│                                                                                 │
│  Score = 0.40 × (Preference Match)                                              │
│        + 0.35 × (Similar to Liked Games)                                        │
│        + 0.15 × (Quality Score)                                                 │
│        + 0.10 × (Exploration Factor)                                            │
│                                                                                 │
│  Output: 0.0 to 1.0 (displayed as 0% to 100%)                                   │
│                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────┘
```

#### User Personas

| Persona           | Description                                   | Primary Need                              |
| ----------------- | --------------------------------------------- | ----------------------------------------- |
| **Casual Gamer**  | Plays occasionally, owns 20-50 games          | Quick game selection, simple interface    |
| **Enthusiast**    | Plays daily, owns 100+ games across platforms | Unified library, playtime stats           |
| **Completionist** | Tracks achievements, completion rates         | Progress tracking, similar game discovery |

---

### 7. Feasibility and Risk Analysis

#### Technical Feasibility

| Aspect      | Assessment | Details                                           |
| ----------- | ---------- | ------------------------------------------------- |
| Team Skills | Feasible   | C++, Python, Qt knowledge present in team         |
| Hardware    | Feasible   | Standard development machines sufficient          |
| Software    | Feasible   | All tools are free/open-source                    |
| Timeline    | Feasible   | 10-12 weeks requires disciplined scope management |

#### Risk Analysis

| Risk                                | Probability | Impact | Mitigation                                  |
| ----------------------------------- | ----------- | ------ | ------------------------------------------- |
| Platform API limitations            | Medium      | High   | Focus on manual game addition for MVP       |
| ML model accuracy issues            | Medium      | Medium | Start with simple models, iterate           |
| Integration complexity (C++↔Python) | Medium      | Medium | Use JSON file-based communication initially |
| Scope creep                         | High        | High   | Strict definition, feature freezes          |
| Team coordination issues            | Low         | Medium | Weekly syncs, clear role ownership          |

---

### 8. Team Responsibilities

| Member             | Role                            | Primary Responsibilities                                                                                                |
| ------------------ | ------------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| Keerti Vardhan     | Backend & Systems Developer     | Develop the C++ core logic for game execution, process monitoring, and cross-platform detection.                        |
| Yuvraj Bhardwaj    | ML & Algorithm Developer        | Design the session analytics pipeline and implement algorithms for temporal usage patterns and peak play hour analysis. |
| Ayushmaan Kapruwan | Frontend & UI Developer         | Design and implement responsive UI components using Qt 6 and QML for library visualization and stats display.           |
| Aditya Kediyal     | Database & Integration Engineer | Manage SQLite local storage, design data schemas for session logs, and integrate external IGDB API data.                |

---
### 9. Project Timeline

```
┌──────────────────────────────────────────────────────────────────────────────
│                           12-WEEK PROJECT TIMELINE                           │
├──────────────────────────────────────────────────────────────────────────────
│                                                                              │
│  PHASE 1: FOUNDATION (Weeks 1-3)                                             │
│  ├── Week 1: Requirements finalization, environment setup                    │
│  ├── Week 2: Database schema design, project structure                       │
│  └── Week 3: Basic UI shell, core data models                                │
│                                                                              │
│  PHASE 2: CORE DEVELOPMENT (Weeks 4-7)                                       │
│  ├── Week 4: Game library manager, add/edit/delete games                     │
│  ├── Week 5: Session tracking, playtime logging                              │
│  ├── Week 6: Like/Dislike system, preference storage                         │
│  └── Week 7: Python ML module setup, feature extraction                      │
│                                                                              │
│  PHASE 3: ML INTEGRATION (Weeks 8-10)                                        │
│  ├── Week 8-9: Content-based recommendation model                              │
│  └── Week 10: C++↔Python integration, recommendation display                 │
│                                                                              │
│  PHASE 4: FINALIZATION (Weeks 11-12)                                         │
│  ├── Week 11: Testing, bug fixes, evaluation metrics                         │
│  └── Week 12: Documentation, presentation, viva preparation                  │
│                                                                              │
└──────────────────────────────────────────────────────────────────────────────
```

#### Gantt Chart

| Task                 | W1  | W2  | W3  | W4  | W5  | W6  | W7  | W8  | W9  | W10 | W11 | W12 |
| -------------------- | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| Requirements & Setup | ██  | ██  |     |     |     |     |     |     |     |     |     |     |
| Database Design      |     | ██  | ██  |     |     |     |     |     |     |     |     |     |
| UI Development       |     |     | ██  | ██  | ██  | ██  |     |     |     |     |     |     |
| Backend Core         |     |     | ██  | ██  | ██  | ██  |     |     |     |     |     |     |
| Data Pipeline        |     |     |     |     | ██  | ██  | ██  |     |     |     |     |     |
| ML Development       |     |     |     |     |     |     | ██  | ██  | ██  |     |     |     |
| Integration          |     |     |     |     |     |     | ██  | ██  | ██  | ██  |     |     |
| Testing              |     |     |     |     |     |     |     |     | ██  | ██  | ██  |     |
| Documentation        |     |     |     |     |     |     |     |     |     |     | ██  | ██  |

#### PERT Chart

![[image-3 1.png|935x235]]

---

### 10. Expected Outcomes

#### Deliverables

| Deliverable              | Description                                                              |
| ------------------------ | ------------------------------------------------------------------------ |
| Vortex Application       | Functional Windows desktop application with unified game library         |
| ML Recommendation Engine | Python module providing content-based + mood-aware recommendations       |
| Project Report           | Comprehensive documentation including architecture, methodology, results |
| User Manual              | Guide for installation and usage                                         |
| Source Code              | Well-documented codebase on GitHub                                       |

#### Success Metrics

| Metric                               | Target |
| ------------------------------------ | ------ |
| Recommendation Precision             |        |
| Application Startup Time             |        |
| Memory Usage                         |        |
| User Satisfaction (internal testing) | > 7/10 |

#### Academic Contributions

1. Demonstration of mood-aware recommendation in gaming context
2. Practical implementation of hybrid ML pipeline in native application
3. Modular architecture pattern for feature-rich desktop applications


---

## 11. References

1. M. Kozakov and N. Kozakova, “DEVELOPMENT OF A RECOMMENDATION SYSTEM FOR VIDEO GAMES”, GoS, no. 58, pp. 814–819, Nov. 2025. 
 ^c03d51
2. https://api.playnite.link/docs/manual/gettingStarted/gettingStartedOverview.html
^de10d3
3. https://docs.gog.com/quick-start/
^6db295
4. https://partner.steamgames.com/doc/home
^9a6fda
5. https://www.igdb.com/api ^7e8bd0

---