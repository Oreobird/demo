# !/usr/bin/env python
# -*- coding:utf-8 -*-
# date: 2019/3/15
# author: zgs

import os
import numpy as np
from sklearn import model_selection
from mlxtend.classifier import StackingClassifier
from sklearn.model_selection import GridSearchCV

import metrics_wrapper

import lightgbm

class AutoML:
    def __init__(self, model_helper=None, model_save_path='./'):
        if model_helper == None:
            raise ValueError("model helper is None")
        self.model_helper = model_helper
        self.model_save_path = model_save_path
        self.selected_models = []
        
    def __print_best_score(self, gs, param_test):
        print("Best score: %0.3f" % gs.best_score_)
        print("Best parameters set:")
    
        best_parameters = gs.best_estimator_.get_params()
        for param_name in sorted(param_test.keys()):
            print("\t%s: %r" % (param_name, best_parameters[param_name]))
            
            
    def cvselect_model(self, x_train, y_train, metric_list, model_label_list, K=3, cv=5):
        select_models = []
        for metric in metric_list:
            scores = {}
            print("===>[%s]" % metric)
            for model_label in model_label_list:
                model = self.model_helper.get_model(model_label)
                score = model_selection.cross_val_score(model, x_train, y_train, cv=cv, scoring=metric)
                scores[model_label] = score.mean()
                print("    [%s] Accuracy: %0.2f (+/- %0.2f)" % (model_label, score.mean(), score.std()))
            sorted_scores = sorted(scores.items(), key=lambda d:d[1], reverse = True)
            selected_model_label = sorted_scores[:K]
            
            for i in range(len(selected_model_label)):
                if not selected_model_label[i][0] in select_models:
                    select_models.append(selected_model_label[i][0])
        
        self.selected_models = select_models[:K]
        return self.selected_models[:K]


    def model_param_tune(self, x_train, y_train, model, model_params, scoring='accuracy', cv=5):
        gs = GridSearchCV(estimator=model, param_grid=model_params, scoring=scoring, cv=cv)
        gs.fit(x_train, y_train)
        best_model = gs.best_estimator_
        self.__print_best_score(gs, model_params)
        return best_model


    def tune_models(self, x_train, y_train, model_labels=None, save=True):
        best_models = {}
        for model_label in model_labels:
            model = self.model_helper.get_model(model_label)
            param_set = self.model_helper.get_param_set(model_label)
            best_model = self.model_param_tune(x_train, y_train, model, param_set)
            if save:
                self.model_helper.save_model(best_model, './' + model_label + '_model.pkl')
            best_models[model_label] = best_model
        return best_models


    def get_basic_models(self, models):
        classifiers = []
        for model in models:
            model_path = os.path.join(self.model_save_path, model + '_model.pkl')
            if not os.path.exists(model_path):
                classifiers.append(get_model(model))
            else:
                basic_model = self.model_helper.load_model(model_path)
                print("Load model:" + model)
                classifiers.append(basic_model)
        return classifiers
    
    
    def stacking(self, x_train, y_train, meta_clf_label=None, save=True):
        meta_clf = self.model_helper.get_meta_model(meta_clf_label)
        meta_param = self.model_helper.get_meta_param_set(meta_clf_label)
        
        stack_clf = StackingClassifier(classifiers=self.get_basic_models(self.selected_models),
                                       use_probas=False,
                                       average_probas=False,
                                       meta_classifier=meta_clf)
    
        best_model = self.model_param_tune(x_train, y_train, stack_clf, meta_param)
        if save:
            self.model_helper.save_model(best_model, os.path.join(self.model_save_path, 'stack_model.pkl'))
        return best_model
    
    def train(self, x_train, y_train, metric_list, model_label_list, meta_model_label, K=3):
        self.cvselect_model(x_train, y_train, metric_list, model_label_list, K=K)
    
        print("Selected models:{}".format(self.selected_models))
    
        self.tune_models(x_train, y_train, self.selected_models)
        return self.stacking(x_train, y_train, meta_model_label)

    def validate(self, model, x_val, y_val, metric_type=None):
        predict_y = model.predict(x_test)
        metrics = metrics_wrapper.MetricsWrapper()
        print(metrics.sk_accuracy(y_test, predict_y))
        return predict_y

    def predict(self, model, x_test, y_test=None, metric_type=None):
        predict_y = model.predict(x_test)
        # metrics = metrics_wrapper.MetricsWrapper()
        # print(metrics.sk_accuracy(y_test, predict_y))
        return predict_y


