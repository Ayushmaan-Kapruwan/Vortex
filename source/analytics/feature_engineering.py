import numpy as np
import pandas as pd

def compute_interest_score(row):
    frequency = row["frequency"]
    duration = row["avg_duration"]
    ratio = row["avg_activity_ratio"]
    recency = row["recency"]

    return (
        0.4 * ratio +
        0.3 * frequency +
        0.2 * duration +
        0.1 * recency
    )

def compute_trend(series):
    x = np.arange(len(series))
    y = np.array(series)

    slope = np.polyfit(x, y, 1)[0]

    return slope