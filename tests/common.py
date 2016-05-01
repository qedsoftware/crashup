#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
import sys
import shutil
import os
import functools

from .widget_tracker import WidgetTracker


def runapp():
    if sys.platform.startswith('linux'):
        # on linux there is support for virtual displays, so headless=True
        return WidgetTracker("build/demoapp", headless=True)
    elif sys.platform.startswith('win'):
        # don't create virtual display on windows
        return WidgetTracker("build\Debug\demoapp.exe", headless=False)
    else:
        raise Exception("Unsupported platform!")


def cleanup_minidumps(func):
    assert hasattr(func, '__call__')
    assert not isinstance(func, type)
    @functools.wraps(func)
    def newfunc(*args, **kwargs):
        if os.path.exists("minidumps"):
            shutil.rmtree("minidumps")
        assert not os.path.exists("minidumps")
        try:
            res = func(*args, **kwargs)
        finally:
            if os.path.exists("minidumps"):
                shutil.rmtree("minidumps")
        return res
    return newfunc
