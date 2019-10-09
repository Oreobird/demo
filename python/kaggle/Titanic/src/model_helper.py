# !/usr/bin/env python
# -*- coding:utf-8 -*-
# date: 2019/3/15
# author: zgs

from sklearn.linear_model import LogisticRegression
from sklearn.svm import SVC
from sklearn.neighbors import KNeighborsClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import ExtraTreesClassifier
import lightgbm
import xgboost

from sklearn.cluster import KMeans

from sklearn.externals import joblib

# from pyspark.mllib.classification import LogisticRegressionWithSGD
# from pyspark.mllib.classification import LogisticRegressionWithLBFGS
# from pyspark.mllib.classification import SVMWithSGD
# from pyspark.mllib.classification import SVMModel
# from pyspark.mllib.classification import NaiveBayes
# from pyspark.mllib.classification import NaiveBayesModel
#
# from pyspark.mllib.clustering import KMeans
# from pyspark.mllib.clustering import KMeansModel
# from pyspark.mllib.clustering import GaussianMixture
# from pyspark.mllib.clustering import GaussianMixtureModel
#
#
# from pyspark.mllib.tree import DecisionTree
# from pyspark.mllib.tree import DecisionTreeModel
# from pyspark.mllib.tree import RandomForest
# from pyspark.mllib.tree import RandomForestModel
# from pyspark.mllib.tree import GradientBoostedTrees
# from pyspark.mllib.tree import GradientBoostedTreesModel
#
# from pyspark.ml.classification import LogisticRegression


default_model_dict = {'lr': [LogisticRegression(multi_class='auto', solver='lbfgs', penalty='l2', verbose=0),
                             {'C': [x / 10.0 for x in range(1, 50, 5)],
                              'max_iter': [50, 100, 500],
                              'multi_class': ['auto'],
                              'solver': ['newton-cg', 'lbfgs', 'liblinear', 'sag', 'saga'],
                              'class_weight': ['balanced']}],
        
                      'decisiontree': [DecisionTreeClassifier(),
                                       {
                                           'max_depth': range(2, 8, 1),
                                           'min_samples_split': [x / 10.0 for x in range(1, 5, 2)],
                                           'min_samples_leaf': [x / 10.0 for x in range(1, 5, 2)],
                                           'min_weight_fraction_leaf': [x / 10.0 for x in range(0, 5, 2)],
                                           'min_impurity_decrease': [x / 10.0 for x in range(1, 20, 5)]
                                       }],
        
                      'gaussiannb': [GaussianNB(),
                                     {}],
        
                      'svm': [SVC(gamma='scale'),
                              {'C': [x / 10.0 for x in range(1, 20, 5)]}],
        
                      'knn': [KNeighborsClassifier(),
                              {'n_neighbors': range(2, 10, 2),
                               'leaf_size': range(10, 50, 5)}],
        
                      'randomforest': [RandomForestClassifier(n_estimators=10, random_state=1),
                                       {'n_estimators': range(10, 200, 10),
                                        'max_depth': range(2, 8, 1),
                                        'min_samples_split': [x / 10.0 for x in range(1, 5, 2)],
                                        'min_samples_leaf': [x / 10.0 for x in range(1, 5, 2)],
                                        'min_weight_fraction_leaf': [x / 10.0 for x in range(1, 5, 2)],
                                        'min_impurity_decrease': [x / 10.0 for x in range(1, 20, 5)],
                                        'bootstrap': [True, False]}],
        
                      'xgboost': [xgboost.XGBClassifier(),
                                  {'max_depth': range(4, 12, 2),
                                   'learning_rate': [x / 10.0 for x in range(1, 10, 2)],
                                   'n_estimators': range(50, 100, 20)}],
        
                      'lgbm': [lightgbm.LGBMClassifier(),
                               {'num_leaves': range(2, 10, 2),
                                'max_depth': range(2, 6, 2)}],
        
                      'k-means': [KMeans(),
                                  {'n_clusters': range(2, 20, 2),
                                   'n_init': range(4, 20, 2),
                                   'max_iter': range(200, 400, 100)}]
              }

default_meta_model_dict = {'lgbm': [lightgbm.LGBMClassifier(),
                                    {'meta-lgbmclassifier__num_leaves': range(2, 10, 2),
                                     'meta-lgbmclassifier__max_depth': range(2, 6, 2)}]}

class ModelUtil:
    def __init__(self, model_dict=default_model_dict, meta_model_dict=default_meta_model_dict):
        self.model_dict = model_dict
        self.meta_model_dict = meta_model_dict

    def get_model(self, model_label='randomforest'):
        return self.model_dict[model_label][0]
    
    def get_param_set(self, model_label='randomforest'):
        return self.model_dict[model_label][1]

    def get_meta_model(self, model_label='lgbm'):
        return self.meta_model_dict[model_label][0]
    
    def get_meta_param_set(self, model_label='lgbm'):
        return self.meta_model_dict[model_label][1]
    
    def save_model(self, model, file):
        joblib.dump(model, file)

    def load_model(self, file):
        return joblib.load(file)


