import os
import sys
import joblib
import pandas as pd
import numpy as np
import json
from sklearn.metrics.pairwise import cosine_similarity
from db import get_connection

BASE_DIR = os.path.dirname(__file__)
MODEL_DIR = os.path.join(BASE_DIR, "model")

vectorizer = joblib.load(os.path.join(MODEL_DIR, "vectorizer.pkl"))

mood = int(sys.argv[1]) if len(sys.argv) > 1 else 0

MOOD_GENRES = {
    0: ["Sandbox", "Adventure", "Simulation"],
    1: ["Shooter", "Strategy", "Fighting"],
    2: ["RPG", "Horror", "Story Rich"]
}

conn = get_connection()
games = pd.read_sql("SELECT * FROM games", conn)
sessions = pd.read_sql("SELECT * FROM sessions", conn)
conn.close()

def get_user_id_from_username(username):
    conn = get_connection()
    cur = conn.cursor()
    cur.execute("SELECT user_id FROM users WHERE username=%s", (username,))
    result = cur.fetchone()
    cur.close()
    conn.close()
    if result:
        return result[0]
    return None

def recommend(user_id):
    user_sessions = sessions[sessions["user_id"] == user_id]

    if user_sessions.empty:
        return []

    game_interest = (
        user_sessions
        .groupby("game_id")
        .agg({
            "activity_ratio": "mean",
            "duration_seconds": "mean"
        })
        .reset_index()
    )

    session_counts = (
        user_sessions
        .groupby("game_id")
        .size()
        .reset_index(name="session_count")
    )

    game_interest = game_interest.merge(
        session_counts,
        on="game_id"
    )

    game_interest["interest_score"] = (
        game_interest["activity_ratio"] * 0.5 +
        (game_interest["duration_seconds"] / game_interest["duration_seconds"].max()) * 0.3 +
        (game_interest["session_count"] / game_interest["session_count"].max()) * 0.2
    )

    liked_games = (
        game_interest
        .sort_values("interest_score", ascending=False)["game_id"]
        .tolist()
    )

    liked_metadata = games[games["game_id"].isin(liked_games)].copy()

    if liked_metadata.empty:
        return []

    liked_metadata["combined"] = (
        liked_metadata["genres"].astype(str) + " " +
        liked_metadata["tags"].astype(str)
    )

    games["combined"] = (
        games["genres"].astype(str) + " " +
        games["tags"].astype(str)
    )

    weighted_vectors = []

    for _, row in liked_metadata.iterrows():
        game_id = row["game_id"]

        weight = game_interest.loc[
            game_interest["game_id"] == game_id,
            "interest_score"
        ].values[0]

        vec = vectorizer.transform([row["combined"]]).toarray()[0]

        weighted_vectors.append(vec * weight)

    user_vector = np.mean(weighted_vectors, axis=0).reshape(1, -1)

    all_vectors = vectorizer.transform(games["combined"])

    similarities = cosine_similarity(
        user_vector,
        all_vectors
    )[0]

    games["score"] = similarities

    games["score"] = (
        games["score"] * 0.8 +
        (games["rating"] / 10.0) * 0.2
    )

    preferred_genres = MOOD_GENRES.get(mood, [])

    def mood_bonus(genres):
        genre_list = str(genres)

        for g in preferred_genres:
            if g.lower() in genre_list.lower():
                return 0.15

        return 0.0

    games["score"] += games["genres"].apply(mood_bonus)

    recommendations = games[
        ~games["game_id"].isin(liked_games)
    ].copy()

    recommendations = recommendations.sort_values(
        "score",
        ascending=False
    )

    recommendations = recommendations.drop_duplicates(
        subset=["name"]
    )

    output = []

    for _, row in recommendations.head(10).iterrows():
        output.append({
            "name": row["name"],
            "score": float(row["score"])
        })

    output_path = os.path.join(
        BASE_DIR,
        "recommendations.json"
    )

    with open(output_path, "w") as f:
        json.dump(output, f, indent=4)

if __name__ == "__main__":
    username = "user1"
    user_id = get_user_id_from_username(username)

    if user_id:
        recommend(user_id)