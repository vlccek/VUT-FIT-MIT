import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

from imblearn.pipeline import Pipeline as ImbPipeline
from imblearn.over_sampling import ADASYN

from sklearn.metrics import confusion_matrix, classification_report
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import StratifiedKFold, cross_validate, train_test_split, GridSearchCV

from sklearn.neural_network import MLPClassifier
from sklearn.feature_selection import SelectKBest, mutual_info_classif


df = pd.read_csv(
    "zzn.csv",
    dtype={
        "Timestamp": str,
        "Your Academic Stage": str,
        "Peer pressure": int,  # i
        "Academic pressure from your home": int,   # i
        "Study Environment": str,
        "What coping strategy you use as a student?": str,
        "Do you have any bad habits like smoking, drinking on a daily basis?": str,
        "What would you rate the academic  competition in your student life": int,   # i
        "Rate your academic stress index ": int,
    }
)

df.dropna(subset=df.columns, inplace=True)
df = df.drop("Timestamp", axis=1)

df.columns = [
    'Academic_stage', 
    'Peer_pressure', 
    'Academic_pressure_home',
    'Study_environment',
    "Coping_strategy",
    'Bad_habits',
    'Academic_competition_rating', 
    'Academic_stress_index', 
]

df = pd.get_dummies(df, drop_first=True)
df["Academic_stress_index"] = df["Academic_stress_index"].apply(lambda x: 0 if x in [1, 2, 3] else 1)

y = df['Academic_stress_index']
X = df.drop(columns=['Academic_stress_index']) 

# split data into training (internal train set + internal test validation set) + confusion matrix set, 
X_train, X_test, y_train, y_test = train_test_split(
    X, 
    y, 
    test_size=0.25, 
    random_state=42, 
    stratify=y
)

cv_strategy = StratifiedKFold(n_splits=5, shuffle=True, random_state=42)

clf = ImbPipeline([
    ('adasyn', ADASYN(random_state=42)),
    ('scaler', StandardScaler()),
    ('feature_selection', SelectKBest(
        score_func=mutual_info_classif,
    )),
    ('classification', MLPClassifier(
        random_state=42, 
        max_iter=1000,
        early_stopping=True,
        n_iter_no_change=20,
        solver="adam",
    )),
])

param_grid = {
    'adasyn__n_neighbors': [3, 4],
    'feature_selection__k': [9, 10, 11], 
    'classification__hidden_layer_sizes': [(19,)],  # (20,), (10,5), (15,5), (15,), (25,)
    'classification__activation': ['relu', 'tanh'],
    'classification__alpha': [0.0001, 0.001],
}

grid_search = GridSearchCV(
    estimator=clf,
    param_grid=param_grid,
    cv=cv_strategy, 
    scoring='recall_macro',  # f1_macro
    n_jobs=-1,
    verbose=0
)

print("Running grid search on X_train, y_train...")

# train on 75% only
grid_search.fit(X_train, y_train) 

# output
best_clf = grid_search.best_estimator_
selector = best_clf.named_steps['feature_selection']
vybrane_sloupce = X.columns[selector.get_support()]

print(f"Features before: {X.shape[1]}")
print(f"Features after: {len(vybrane_sloupce)}")
print(f"Selected features: {list(vybrane_sloupce)}")
print("Best params: ", grid_search.best_params_)

# crossvalidation
scoring_metrics = [
    'accuracy', 
    'precision_macro', 
    'recall_macro', 
    'f1_macro', 
    'f1_weighted',
]
results = cross_validate(
    best_clf, 
    X_train,
    y_train,
    cv=cv_strategy, 
    scoring=scoring_metrics,
    return_train_score=False 
)

print("Mean accuracy: {:.4f} (+/- {:.4f})".format(
    results['test_accuracy'].mean(), 
    results['test_accuracy'].std()
))
print("Avg F1 CV: {:.4f}".format(results['test_f1_macro'].mean()))
print("Avg F1 (weighted) CV: {:.4f}".format(results['test_f1_weighted'].mean()))

# CM visual
y_pred_test = best_clf.predict(X_test)
cm = confusion_matrix(y_test, y_pred_test)
plt.figure(figsize=(6, 5))
sns.heatmap(
    cm, 
    annot=True, 
    fmt='d', 
    cmap='Blues',
    xticklabels=['Nízký stres (0)', 'Vysoký stres (1)'],
    yticklabels=['Nízký stres (0)', 'Vysoký stres (1)']
)
plt.title('Matice záměn (Valid. Sada)')
plt.ylabel('Skutečná třída')
plt.xlabel('Predikovaná třída')
plt.show()

# classification_report
print(classification_report(
    y_test, 
    y_pred_test, 
    target_names=['Nízký stres (0)', 'Vysoký stres (1)']
))
