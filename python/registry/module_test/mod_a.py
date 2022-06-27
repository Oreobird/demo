# !/usr/bin/env python
# -*- coding:utf-8 -*-

from registry import register_obj

@register_obj
class TestObjA:
    def __init__(self):
        print("This is TestObj A")

@register_obj("TestObjB")
class TestObjB:
    def __init__(self):
        print("This is TestObj B")