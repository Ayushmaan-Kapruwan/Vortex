import requests
import uuid
from db import get_connection

def insert_game(
    name,
    source,
    appid=None,
    genres=None,
    tags=None,
    game_length=0,
    rating=0,
    review_count=0
):
    conn = get_connection()
    cur = conn.cursor()

    cur.execute("""
        INSERT INTO games (
            game_id, source, name, appid,
            genres, tags, game_length,
            rating, review_count
        )
        VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s)
    """, (
        str(uuid.uuid4()),
        source,
        name,
        appid,
        genres,
        tags,
        game_length,
        rating,
        review_count
    ))

    conn.commit()
    cur.close()
    conn.close()