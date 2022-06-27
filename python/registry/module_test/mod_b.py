# !/usr/bin/env python
# -*- coding:utf-8 -*-

from registry import register_func

@register_func
def test_func_1():
    print("This is test func 1")

@register_func("test_func_2")
def test_func_2():
    print("This is test func 2")