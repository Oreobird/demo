from sklearn.model import train_test_split
from sklearn.ensemble import GradientBoostingClassifier
from sklearn.preprocessing import OneHotEncoder
from sklearn.linear_model import LogisticRegression

X_gdbdt, X_lr, y_gbdt, y_lr = train_test_split(X, y, test_size=0.5)
gbdt = GradientBoostingClassifier()
gbdt.fit(X_gbdt, y_gbdt)

leaves = gbdt.apply(X_lr)[:,:,0]

features_trans = OneHotEncoder.fit_transform(leaves)

lr = LogisticRegression()
lr.fit(features_trans, y_lr)

lr.predict(features_trans)
lr.predict_proba(features_trans)[:,1]
