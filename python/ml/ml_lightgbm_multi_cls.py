import lightgbm as lgb
import pandas as pd
import numpy as np
from sklearn import datasets
import pickle
from sklearn.metrics import roc_auc_score,accuracy_score
from sklearn.model_selection import train_test_split

print("Loading Data ... ")
# 导入数据
iris = datasets.load_iris()

# 用sklearn.cross_validation进行训练数据集划分，这里训练集和交叉验证集比例为7：3，可以自己根据需要设置
X, val_X, y, val_y = train_test_split(iris.data, iris.target,
                                      test_size=0.3,
                                      random_state=1,
                                      stratify=iris.target)

X_train = X
y_train = y

X_test = val_X
y_test = val_y

# create dataset for lightgbm
lgb_train = lgb.Dataset(X_train, y_train)
lgb_eval = lgb.Dataset(X_test, y_test, reference=lgb_train)

# specify your configurations as a dict
params = {
    'boosting_type': 'gbdt',
    'objective': 'multiclass',
    'num_class': 3,
    'metric': 'multi_error',
    'num_leaves': 80,
    'max_depth': 7,
    'min_data_in_leaf': 10,
    'learning_rate': 0.01,
    'feature_fraction': 0.8,
    'bagging_fraction': 0.8,
    'bagging_freq': 5,
    'lambda_l1': 0.1,
    'lambda_l2': 0.2,
}

# train
print('Start training...')
gbm = lgb.train(params,
                lgb_train,
                num_boost_round=10000,
                valid_sets=lgb_eval,
                early_stopping_rounds=500)

print('Start predicting...')
preds = gbm.predict(X_test, num_iteration=gbm.best_iteration) # 输出的是概率结果

preds = [list(x).index(max(x)) for x in preds]
print(accuracy_score(y_test, preds))

# 导出结果
# for pred in preds:
#     result = prediction = int(np.argmax(pred))
#
#     # 导出特征重要性
#     importance = gbm.feature_importance()
#     names = gbm.feature_name()
#
#     with open('./feature_importance.txt', 'w+') as file:
#         for index, im in enumerate(importance):
#             string = names[index] + ', ' + str(im) + 'n'
#             file.write(string)