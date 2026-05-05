import pandas as pd
import joblib
from sklearn.decomposition import TruncatedSVD
from sklearn.feature_extraction.text import TfidfVectorizer

from db import get_connection

conn = get_connection()

games = pd.read_sql("SELECT * FROM games", conn)
sessions = pd.read_sql("SELECT * FROM sessions", conn)

games["combined"] = games["genres"].astype(str) + " " + games["tags"].astype(str)

vectorizer = TfidfVectorizer()
game_vectors = vectorizer.fit_transform(games["combined"])

pivot = sessions.pivot_table(
    index="user_id",
    columns="game_id",
    values="activity_ratio",
    aggfunc="mean"
).fillna(0)

svd = TruncatedSVD(n_components=20)
user_game_matrix = svd.fit_transform(pivot)

joblib.dump(vectorizer, "C:\\Users\\yuvra\\OneDrive\\Dokumen\\minor\\Vortex-main\\source\\analytics\\model\\vectorizer.pkl")
joblib.dump(svd, "C:\\Users\\yuvra\\OneDrive\\Dokumen\\minor\\Vortex-main\\source\\analytics\\model\\svd.pkl")

print("Model trained.")