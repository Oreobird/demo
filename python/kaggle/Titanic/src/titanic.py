import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib
import os
import matplotlib.pyplot as plt
import warnings
warnings.filterwarnings('ignore')
import sklearn
from sklearn import metrics
from sklearn.svm import SVC
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import export_graphviz, DecisionTreeClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import cross_val_score, train_test_split
from sklearn.exceptions import NotFittedError
from sklearn.base import BaseEstimator, ClassifierMixin
import lightgbm as lgb
from sklearn.model_selection import GridSearchCV

from IPython.display import display

pd.set_option('display.max_columns',1000)
pd.set_option('display.width', 1000)
pd.set_option('display.max_colwidth',1000)
np.set_printoptions(threshold=np.nan)

path_train = '../data/train.csv'
path_test = '../data/test.csv'

def data_explor(df_raw):
    print(df_raw.head())
    
    print(df_raw.describe())
    print(df_raw.info())
    
    plot = sns.catplot(x="Embarked", y="Fare", hue="Sex", data=df_raw, height=6, kind="bar", palette="muted")
    plot.set_ylabels("Pclass")
    plt.show()
    
    embarked_null = df_raw[df_raw['Embarked'].isnull()]
    print(embarked_null)

    df_raw.drop(['PassengerId'], 1).hist(bins=50, figsize=(20, 15))
    plt.show()

def preprocess_data(df):
    process_df = df
    
    #1.Deal with missing values
    
    process_df['Embarked'].fillna('C', inplace=True)
    
    # replace missing age by the mean age of passengers who belong to the same group of class/sex/family
    process_df['Age'] = process_df.groupby(['Pclass', 'Sex', 'Parch', 'SibSp'])['Age'].transform(lambda x: x.fillna(x.mean()))
    process_df['Age'] = process_df.groupby(['Pclass', 'Sex', 'Parch'])['Age'].transform(lambda x: x.fillna(x.mean()))
    process_df['Age'] = process_df.groupby(['Pclass', 'Sex'])['Age'].transform(lambda x: x.fillna(x.mean()))
    
    # replace the only missing fare value for test dataset and the missing values of the cabin column
    process_df['Fare'] = process_df['Fare'].interpolate()
    process_df['Cabin'].fillna('U', inplace=True)
    
    #2. Feature engineeing on columns
    # Create a title column from name column
    process_df['Title'] = pd.Series((name.split('.')[0].split(',')[1].strip() for name in train_df_raw['Name']),
                                      index=train_df_raw.index)
    process_df['Title'] = process_df['Title'].replace(
        ['Lady', 'the Countess', 'Countess', 'Capt', 'Col', 'Don', 'Dr', 'Major', 'Rev', 'Sir', 'Jonkheer', 'Dona'],
        'Rare')
    process_df['Title'] = process_df['Title'].replace(['Mlle', 'Ms'], 'Miss')
    process_df['Title'] = process_df['Title'].replace('Mme', 'Mrs')
    process_df['Title'] = process_df['Title'].map({"Mr": 1, "Miss": 2, "Mrs": 3, "Master": 4, "Rare": 5})
    
    # Filling Age missing values with mean age of passengers who have the same title
    process_df['Age'] = process_df.groupby(['Title'])['Age'].transform(lambda x: x.fillna(x.mean()))
    
    # print(process_df['Age'])
    
    # Transform categorical variables to numeric variables
    process_df['Sex'] = process_df['Sex'].map({'male': 0, 'female': 1})
    process_df['Embarked'] = process_df['Embarked'].map({'S': 0, 'C': 1, 'Q': 2})
    
    # Create a Family size, Is alone, child and mother columns
    process_df['FamillySize'] = process_df['SibSp'] + process_df['Parch'] + 1
    process_df['FamillySize'][process_df['FamillySize'].between(1, 5, inclusive=False)] = 2
    process_df['FamillySize'][process_df['FamillySize']>5] = 3
    process_df['IsAlone'] = np.where(process_df['FamillySize']!=1, 0, 1)
    process_df['IsChild'] = process_df['Age'] < 18
    process_df['IsChild'] = process_df['IsChild'].astype(int)
    
    # Modification of cabin to keep only the letter contained corresponding to the deck of the boat
    process_df['Cabin'] = process_df['Cabin'].str[:1]
    process_df['Cabin'] = process_df['Cabin'].map({cabin: p for p, cabin in enumerate(set(cab for cab in process_df['Cabin']))})
    
    # Create a ticket survivor column
    process_df['TicketSurvivor'] = pd.Series(0, index=process_df.index)
    tickets = process_df['Ticket'].value_counts().to_dict()
    for t, occ in tickets.items():
        if occ != 1:
            table = train_df_raw['Survived'][train_df_raw['Ticket'] == t]
            if sum(table) != 0:
                process_df['TicketSurvivor'][process_df['Ticket'] == t] = 1
    
    # drop not useful anymore
    process_df = process_df.drop(['Name', 'Ticket', 'PassengerId'], 1)
    
    return process_df
    
    
train_df_raw = pd.read_csv(path_train)
test_df_raw = pd.read_csv(path_test)

# data_explor(df_raw)

train_df = train_df_raw.copy()

X = train_df.drop(['Survived'], 1)
Y = train_df['Survived']

X = preprocess_data(X)
print(X)
sc = StandardScaler()
X = pd.DataFrame(sc.fit_transform(X.values), index=X.index, columns=X.columns)

X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.2, random_state=42)

# print(X_train.head())

# 1. LR
# lg = LogisticRegression(solver='lbfgs', random_state=42)
# lg.fit(X_train, Y_train)
# logistic_prediction = lg.predict(X_test)
# score = metrics.accuracy_score(Y_test, logistic_prediction)
# # display_confusion_matrix(Y_test, logistic_prediction, score=score)
#
# # 2. Decistion Tree
# dt = DecisionTreeClassifier(min_samples_split=15, min_samples_leaf=20, random_state=42)
# dt.fit(X_train, Y_train)
# dt_prediction = dt.predict(X_test)
# score = metrics.accuracy_score(Y_test, dt_prediction)
# # display_confusion_matrix(Y_test, dt_prediction, score=score)
# # visualize_tree(dt, X_test.columns)
#
# #3. SVM
# svm = SVC(gamma='auto', random_state=42)
# svm.fit(X_train, Y_train)
# svm_prediction = svm.predict(X_test)
# score = metrics.accuracy_score(Y_test, svm_prediction)
# # display_confusion_matrix(Y_test, svm_prediction, score=score)
#
# #4. Random forest
# rf = RandomForestClassifier(n_estimators=200, random_state=42)
# rf.fit(X_train, Y_train)
# rf_prediction = rf.predict(X_test)
# score = metrics.accuracy_score(Y_test, rf_prediction)
# # display_confusion_matrix(Y_test, rf_prediction, score=score)
#
# n_folds = 10
# cv_score_lg = cross_val_score(estimator=lg, X=X_train, y=Y_train, cv=n_folds)
# cv_score_dt = cross_val_score(estimator=dt, X=X_train, y=Y_train, cv=n_folds)
# cv_score_svm = cross_val_score(estimator=svm, X=X_train, y=Y_train, cv=n_folds)
# cv_score_rf = cross_val_score(estimator=rf, X=X_train, y=Y_train, cv=n_folds)
#
# cv_result = {'lg':cv_score_lg,'dt':cv_score_dt, 'svm':cv_score_svm, 'rf':cv_score_rf}
# cv_data = {model:[score.mean(), score.std()] for model, score in cv_result.items()}
# cv_df = pd.DataFrame(cv_data, index=['Mean_accuracy', 'Variance'])
# print(cv_df)
#
# plt.figure(figsize=(20,8))
# plt.plot(cv_result['lg'])
# plt.plot(cv_result['dt'])
# plt.plot(cv_result['svm'])
# plt.plot(cv_result['rf'])
# plt.title('Models Accuracy')
# plt.ylabel('Accuracy')
# plt.xlabel('Trained fold')
# plt.xticks([k for k in range(n_folds)])
# plt.legend(['logreg', 'tree', 'randomforest', 'svm'], loc='upper left')
# plt.show()


class EsemblingClassifer:
    def __init__(self, verbose=True):
        self.dt = DecisionTreeClassifier(min_samples_split=15, min_samples_leaf=20, random_state=42)
        self.rf = RandomForestClassifier(n_estimators=300, max_depth=11, random_state=42)
        self.svm = SVC(random_state=42)
        self.gbm_params = {
            'boosting_type': 'gbdt', # 提升类型
            'objective': 'binary',  # 目标函数
            'metric': {'binary_logloss', 'auc'}, # 评估函数
            'num_leaves': 5,  #叶子节点数
            'max_depth': 6,     # 最大深度
            'min_data_in_leaf': 70,
            'learning_rate': 0.1,  #学习率
            'feature_fraction': 0.9,  #建树的特征选择比例
            'bagging_fraction': 0.95, #建树的样本采样比例
            'bagging_freq': 5,  # 每 5 次迭代执行bagging
            'lambda_l1': 1,
            'lambda_l2': 0.001, # 越小l2正则程度越高
            'min_gain_to_split': 0.2,
            'verbose': 5,
            'is_unbalance': False
        }
        
        self.gbm = None
        self.trained = False
        self.verbose = verbose
        
    def fit(self, X, y):
        if self.verbose:
            print('--------Fitting models---------')
        self.dt.fit(X, y)
        self.rf.fit(X, y)
        self.svm.fit(X, y)
        # X_train, X_val, y_train, y_val = train_test_split(X, y, test_size=0.2, random_state=42)
        # lgb_train = lgb.Dataset(X_train, y_train)
        # lgb_val = lgb.Dataset(X_val, y_val)
        lgb_train = lgb.Dataset(X, y)
        self.gbm = lgb.train(params=self.gbm_params, train_set=lgb_train,
                            # valid_sets=lgb_val,
                            num_boost_round=5000)
        self.trained = True
        
    def predict(self, X):
        if self.trained == False:
            raise NotFittedError('Please train the classifier before making a prediction')
        if self.verbose:
            print('---------Making and combining predictions----------')
        predictions = list()
        dt_pred = self.dt.predict(X)
        rf_pred = self.rf.predict(X)
        svm_pred = self.svm.predict(X)
        lgb_pred = self.gbm.predict(X, num_iteration=self.gbm.best_iteration)
        lgb_pred = np.where(lgb_pred > 0.5, 1, 0)

        for n in range(len(dt_pred)):
            print("dt{}:{}, rf{}:{}, svm{}:{}, lgb{}:{}".format(n, dt_pred[n], n, rf_pred[n], n, svm_pred[n], n, lgb_pred[n]))
            combined = dt_pred[n] + rf_pred[n] + svm_pred[n] + lgb_pred[n]
            p = 0 if combined == 1 or combined == 0 else 1
            predictions.append(p)
            
        return predictions
    
#
# ens = EsemblingClassifer()
# ens.fit(X_train, Y_train)
# ens_pred = ens.predict(X_test)
# score = metrics.accuracy_score(Y_test, ens_pred)
# print(score)
#
# # ens.fit(X, Y)
#
# submission = pd.DataFrame({
#         "PassengerId": test_df["PassengerId"],
#         "Survived": ens_pred
#     })
# submission.to_csv('../data/submission.csv', index=False)

from sklearn import datasets
from sklearn.model_selection import train_test_split
import model_helper
import config_parser
import metrics_wrapper
import automl_base

cfg = config_parser.CfgParser('E:/data-mining/auto_framework/config.ini')

metric_list, model_label_list = cfg.parse_metrics_models()
meta_model_label = cfg.parse_meta_models()

print(metric_list)
print(model_label_list)
print(meta_model_label)

test_df = test_df_raw.copy()
tx = preprocess_data(test_df)
tx = pd.DataFrame(sc.fit_transform(tx.values), index=tx.index, columns=tx.columns)
print(tx)

model_h = model_helper.ModelHelper()
automl = automl_base.AutoML(model_h)
model = automl.train(X_train, Y_train, metric_list, model_label_list, meta_model_label[0], K=3)


pred_y = automl.predict(model, tx)
print(pred_y)

submission = pd.DataFrame({
        "PassengerId": test_df["PassengerId"],
        "Survived": pred_y
    })
submission.to_csv('../data/submission_new.csv', index=False)
