# !/usr/bin/env python
# -*- coding:utf-8 -*-
# date: 2019/3/15
# author: zgs

class MetricsWrapper1(object):
    def __init__(self, metric, params=None):
        self.metric_fn = metric
        self.params = params
        
    def metric(self):
        self.metric_fn(**self.params)


class MetricsWrapper():
    def __init__(self):
        pass
    
    def sk_accuracy(self, y_true, y_pred, normalize=True, sample_weight=None):
        from sklearn.metrics import accuracy_score
        return accuracy_score(y_true, y_pred, normalize, sample_weight)
    
    def sk_auc(self, x, y, reorder='deprecated'):
        from sklearn.metrics import auc
        return auc(x, y, reorder)
    
    def sk_f1(self, y_true, y_pred, labels=None, pos_label=1, average='binary', sample_weight=None):
        from sklearn.metrics import f1_score
        return f1_score(y_true, y_pred, labels, pos_label, average, sample_weight)
    
    def sk_precision(self, y_true, y_pred, labels=None, pos_label=1, average='binary', sample_weight=None):
        from sklearn.metrics import precision_score
        return precision_score(y_true, y_pred, labels, pos_label,average, sample_weight)
    
    def sk_recall(self, y_true, y_pred, labels=None, pos_label=1, average='binary', sample_weight=None):
        from sklearn.metrics import recall_score
        return recall_score(y_true, y_pred, labels, pos_label, average, sample_weight)
    
    def sk_roc_auc(self, y_true, y_score, average="macro", sample_weight=None, max_fpr=None):
        from sklearn.metrics import roc_auc_score
        return roc_auc_score(y_true, y_score, average, sample_weight, max_fpr)
    
    def sk_confusion_matrix(self, y_true, y_pred, labels=None, sample_weight=None):
        from sklearn.metrics import confusion_matrix
        return confusion_matrix(y_true, y_pred, labels, sample_weight)
    
    def sp_accuracy(self, prediction_and_labels):
        from pyspark.mllib.evaluation import MulticlassMetrics
        return MulticlassMetrics(prediction_and_labels)
    
    
    
    
        



