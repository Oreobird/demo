# !/usr/bin/env python
# -*- coding:utf-8 -*-
# date: 2019/3/15
# author: zgs

import configparser

class CfgParser():
    def __init__(self, cfg_file):
        self.parser = configparser.ConfigParser()
        self.parser.read(cfg_file)

    def parse_type(self):
        cfg_prob_type = 'binary'
        cfg_prob_type = self.parser.get('basic', 'problem_type')
        return cfg_prob_type
    
    def parse_clf_metrics(self):
        metrics_list = []
        for item, value in self.parser.items('clf_metrics'):
            if value == 'true':
                metrics_list.append(item)
        return metrics_list

    def parse_clf_models(self):
        models_list = []
        for item, value in self.parser.items('clf_models'):
            if value == 'true':
                models_list.append(item)
        return models_list

    def parse_meta_clf_model(self):
        models_list = []
        for item, value in self.parser.items('meta_models'):
            if value == 'true':
                models_list.append(item)
        return models_list



