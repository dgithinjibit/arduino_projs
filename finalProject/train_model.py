"""
- Trains a tiny logistic regression model (2 features) using scikit-learn.
- If "sensor_data.csv" exists (with header: f1,f2,label), it will use it.
- Otherwise it generates synthetic example data.
- Prints Arduino-friendly model params to copy into the sketch (weights and bias).

Usage:
  python3 train_model.py
Requirements:
  pip install numpy scikit-learn pandas
"""

import os
import numpy as np
import pandas as pd
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score

CSV = "sensor_data.csv"

def load_or_generate():
    if os.path.exists(CSV):
        df = pd.read_csv(CSV)
        if set(["f1","f2","label"]).issubset(df.columns):
            X = df[["f1","f2"]].values
            y = df["label"].values
            print("Loaded", len(X), "rows from", CSV)
            return X, y
        else:
            print("CSV found but missing required columns 'f1','f2','label'. Generating synthetic data.")
    # synthetic dataset (example)
    rng = np.random.RandomState(1)
    n = 400
    f1 = rng.normal(loc=500, scale=80, size=n)   # e.g., raw analog reading
    f2 = rng.normal(loc=300, scale=60, size=n)   # e.g., moving avg or second sensor
    # label: simple non-linear boundary for realism
    label = ((0.006*f1 + 0.004*f2 + rng.normal(scale=0.2, size=n)) > 4.0).astype(int)
    X = np.vstack([f1, f2]).T
    y = label
    print("Generated synthetic dataset with", n, "samples.")
    return X, y

def train_and_export(X, y):
    scaler = StandardScaler()
    Xs = scaler.fit_transform(X)

    X_train, X_test, y_train, y_test = train_test_split(Xs, y, test_size=0.2, random_state=42)
    clf = LogisticRegression(solver="liblinear")
    clf.fit(X_train, y_train)

    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)

    w = clf.coef_[0]  # two weights
    b = clf.intercept_[0]

    # Because Arduino may not include scaler, export scaler params too
    mean = scaler.mean_
    scale = scaler.scale_

    print("\n=== Model Summary ===")
    print("Test accuracy: {:.3f}".format(acc))
    print("Weights (w0,w1):", w[0], w[1])
    print("Bias:", b)
    print("Scaler mean (m0,m1):", mean[0], mean[1])
    print("Scaler scale (s0,s1):", scale[0], scale[1])

    print("\n=== Arduino-friendly C constants ===")
    print("// paste these into your Arduino sketch or into a model_params.h file")
    print("const float MODEL_W[2] = {{%.8ff, %.8ff}};" % (w[0], w[1]))
    print("const float MODEL_B = %.8ff;" % (b))
    print("const float SCALER_MEAN[2] = {%.8ff, %.8ff};" % (mean[0], mean[1]))
    print("const float SCALER_SCALE[2] = {%.8ff, %.8ff};" % (scale[0], scale[1]))

if __name__ == "__main__":
    X, y = load_or_generate()
    train_and_export(X, y)
