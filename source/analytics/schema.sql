CREATE TABLE users (
    user_id UUID PRIMARY KEY,
    username TEXT UNIQUE NOT NULL
);

CREATE TABLE games (
    game_id UUID PRIMARY KEY,
    source TEXT NOT NULL,
    external_id TEXT,
    name TEXT NOT NULL,
    executable_path TEXT,
    appid INTEGER,
    genres TEXT[],
    tags TEXT[],
    game_length REAL,
    rating REAL,
    review_count INTEGER,
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE sessions (
    session_id UUID PRIMARY KEY,
    user_id UUID REFERENCES users(user_id),
    game_id UUID REFERENCES games(game_id),

    started_at TIMESTAMP,
    ended_at TIMESTAMP,

    duration_seconds INTEGER,
    active_seconds INTEGER,
    idle_seconds INTEGER,

    activity_ratio REAL
);

CREATE TABLE interest_series (
    id UUID PRIMARY KEY,
    user_id UUID REFERENCES users(user_id),
    game_id UUID REFERENCES games(game_id),

    session_number INTEGER,
    interest_value REAL,
    trend_component REAL,

    created_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE recommendations_cache (
    id UUID PRIMARY KEY,
    user_id UUID REFERENCES users(user_id),
    recommended_game UUID REFERENCES games(game_id),
    score REAL,
    created_at TIMESTAMP DEFAULT NOW()
);