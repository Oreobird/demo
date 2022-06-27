# !/usr/bin/env python
# -*- coding:utf-8 -*-

import module_test
from registry import *

def registry_test():
    print("Registered func list: {}".format(get_reg_func_list()))
    print("Registered obj list: {}".format(get_reg_obj_list()))

if __name__ == '__main__':
    registry_test()