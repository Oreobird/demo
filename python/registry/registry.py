# !/usr/bin/env python
# -*- coding:utf-8 -*-

from os.path import basename, isfile, join
import glob

FUNC_DICT = {}
OBJ_DICT = {}

def register_func(func_name, func=None):
    """Register global function

    Parameters
    ----------
    func_name : str or function
        The function name

    func : function, optional
        The function to be registered.

    Returns
    -------
    fregister : function
        Register function if f is not specified.

    Examples
    --------
    The following code registers my_func.

    .. code-block:: python
      targs = (10, "hello")
      @register_func
      def my_func(*args):
          return 10
      # Get it out from register function table
      f = get_register_func("my_func")
      y = f(*targs)
      assert y == 10
    """
    if callable(func_name):
        func = func_name
        func_name = func.__name__

    if not isinstance(func_name, str):
        raise ValueError("expect string function name")

    def register(f):
        FUNC_DICT[func_name] = f
        return f

    if func:
        return register(func)

    return register

def get_reg_func(name):
    """Get a register function by name

    Parameters
    ----------
    name : str
        The name of the registerd function

    Returns
    -------
    fregister : function
        The function to be returned, None if function is missing.
    """
    if name not in FUNC_DICT.keys():
        return None
    return FUNC_DICT[name]

def get_reg_func_list():
    """Get a register function by name

    Returns
    -------
    fregister_list : List
        The function list to be returned, [] if no function registered.
    """
    func_list = []
    for _, v in FUNC_DICT.items():
        func_list.append(v)
    return func_list


def register_obj(obj_type=None):
    """register object type.

    Parameters
    ----------
    obj_type : str or cls
        The name of the node

    Examples
    --------
    The following code registers MyObject
    using type key "test.MyObject"

    .. code-block:: python

      @tvm.register_object("test.MyObject")
      class MyObject(Object):
          pass
    """

    def register(cls):
        obj_name = obj_type if isinstance(obj_type, str) else cls.__name__
        OBJ_DICT[obj_name] = cls
        return cls

    if isinstance(obj_type, str):
        return register

    return register(obj_type)


def get_reg_obj(obj_type):
    """Get a register object by type

    Parameters
    ----------
    obj_type : str
        The obj_type of the register object

    Returns
    -------
    object : str or cls
        The object to be returned, None if object is missing.
    """
    obj_name = obj_type if isinstance(obj_type, str) else obj_type.__name__
    if obj_name not in OBJ_DICT.keys():
        return None
    return OBJ_DICT[obj_name]()

def get_reg_obj_list():
    """Get a register Obj by name

    Returns
    -------
    fregister_list : List
        The Obj list to be returned, [] if no obj registered.
    """
    obj_list = []
    for _, v in OBJ_DICT.items():
        obj_list.append(v)
    return obj_list

def reg_modules(dir):
    """Import all package modules in dir, call in __init__.py of a package

    Parameters
    ----------
    dir : str
        The directory of the

    Returns
    -------
    modules : List
        The modules list to be imported, set to __all__ in __init__.py

    Examples
    --------
    .. code-block:: python
        from os.path import dirname
        from registry import reg_modules
        __all__ = reg_modules(dirname(__file__))
        from . import *
    """
    modules = glob.glob(join(dir, "*.py"))
    return [basename(f)[:-3] for f in modules if isfile(f) and not f.endswith('__init__.py')]
