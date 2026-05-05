import uuid
import random
from datetime import datetime, timedelta

from db import get_connection


GENRES = {
    "RPG": ["Open World", "Fantasy", "Story Rich"],
    "FPS": ["Shooter", "Multiplayer", "Action"],
    "Strategy": ["RTS", "Tactics", "Simulation"],
    "Racing": ["Cars", "Arcade", "Simulation"],
    "Horror": ["Survival", "Psychological", "Dark"],
    "Sandbox": ["Building", "Crafting", "Open World"]
}


GAME_POOL = [
    ("Elden Ring", "RPG"),
    ("Skyrim", "RPG"),
    ("Cyberpunk 2077", "RPG"),
    ("Witcher 3", "RPG"),
    ("CS2", "FPS"),
    ("Valorant", "FPS"),
    ("Call of Duty", "FPS"),
    ("Age of Empires", "Strategy"),
    ("Civilization VI", "Strategy"),
    ("Forza Horizon", "Racing"),
    ("Need for Speed", "Racing"),
    ("Outlast", "Horror"),
    ("Phasmophobia", "Horror"),
    ("Minecraft", "Sandbox"),
    ("Terraria", "Sandbox")
]


USER_PROFILES = {
    "user1": ["RPG"],
    "user2": ["FPS"],
    "user3": ["Strategy"],
    "user4": ["Racing"],
    "user5": ["Horror"],
    "user6": ["Sandbox"],
    "user7": ["RPG", "FPS"],
    "user8": ["Strategy", "Sandbox"]
}


# ---------- USERS ----------
def create_users():
    conn = get_connection()
    cur = conn.cursor()

    for username in USER_PROFILES.keys():
        cur.execute("""
            INSERT INTO users (user_id, username)
            VALUES (%s, %s)
            ON CONFLICT (username) DO NOTHING
        """, (
            str(uuid.uuid4()),
            username
        ))

    conn.commit()
    cur.close()
    conn.close()


# ---------- GAMES ----------
def create_games():
    conn = get_connection()
    cur = conn.cursor()

    for game_name, genre in GAME_POOL:
        tags = GENRES[genre]

        cur.execute("""
            INSERT INTO games (
                game_id,
                source,
                name,
                genres,
                tags,
                game_length,
                rating,
                review_count
            )
            VALUES (%s,%s,%s,%s,%s,%s,%s,%s)
        """, (
            str(uuid.uuid4()),
            "seed",
            game_name,
            [genre],
            tags,
            random.randint(10, 120),
            round(random.uniform(7.0, 10.0), 1),
            random.randint(10000, 500000)
        ))

    conn.commit()
    cur.close()
    conn.close()


# ---------- FETCH ----------
def get_user_ids():
    conn = get_connection()
    cur = conn.cursor()

    cur.execute("SELECT user_id, username FROM users")
    users = cur.fetchall()

    cur.close()
    conn.close()

    return users


def get_games():
    conn = get_connection()
    cur = conn.cursor()

    cur.execute("SELECT game_id, name, genres FROM games")
    games = cur.fetchall()

    cur.close()
    conn.close()

    return games


# ---------- SESSIONS ----------
def create_sessions():
    users = get_user_ids()
    games = get_games()

    conn = get_connection()
    cur = conn.cursor()

    now = datetime.now()
    batch = []

    for user_id, username in users:
        preferred_genres = USER_PROFILES[username]

        
        user_games = random.sample(
            games,
            k=len(games) // 2
        )

        for game_id, game_name, genres in user_games:
            game_genre = genres[0]

            if game_genre in preferred_genres:
                num_sessions = random.randint(5, 12)
                base_activity = random.uniform(0.7, 0.95)
            else:
                num_sessions = random.randint(1, 3)
                base_activity = random.uniform(0.2, 0.5)

            for i in range(num_sessions):
                start = now - timedelta(
                    days=random.randint(1, 30),
                    hours=random.randint(1, 12)
                )

                duration = random.randint(1800, 7200)

                end = start + timedelta(seconds=duration)

                #trend signal
                trend_bonus = i * 0.03

                activity_ratio = min(
                    base_activity + trend_bonus,
                    0.99
                )

                active_seconds = int(duration * activity_ratio)
                idle_seconds = duration - active_seconds

                batch.append((
                    str(uuid.uuid4()),
                    user_id,
                    game_id,
                    start,
                    end,
                    duration,
                    active_seconds,
                    idle_seconds,
                    activity_ratio
                ))

    cur.executemany("""
        INSERT INTO sessions (
            session_id,
            user_id,
            game_id,
            started_at,
            ended_at,
            duration_seconds,
            active_seconds,
            idle_seconds,
            activity_ratio
        )
        VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s)
    """, batch)

    conn.commit()
    cur.close()
    conn.close()


# ---------- MAIN ----------
if __name__ == "__main__":
    print("Seeding users...")
    create_users()

    print("Seeding games...")
    create_games()

    print("Seeding sessions...")
    create_sessions()

    print("Done.")