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
import time
import hashlib

import kibana

from .widget_tracker import WidgetTracker
if (not sys.platform.startswith('win')):
    from fabric.api import env, run, cd, sudo
    from fabric.contrib.files import exists


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
    if sys.platform.startswith('win'):
        DIR = "crashdb"
    else:
        DIR = "minidumps"
    @functools.wraps(func)
    def newfunc(*args, **kwargs):
        if os.path.exists(DIR):
            shutil.rmtree(DIR)
        assert not os.path.exists(DIR)
        try:
            res = func(*args, **kwargs)
        finally:
            if os.path.exists(DIR):
                shutil.rmtree(DIR)
        return res
    return newfunc


def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest().lower()


def wait_until_minidump_is_uploading(timepoint, dumps, timeout=20., step=5.):
    current = 0.
    if sys.platform.startswith('win'):
        minidump_dir = os.path.join("crashdb", "reports")
    elif sys.platform.startswith('linux'):
        minidump_dir = os.path.join("minidumps")
    else:
        raise Exception("This OS is unsupported.")

    minidump_files = [f for f in os.listdir(minidump_dir) if f not in ['.minidumps.json']]
    if len(minidump_files) != 1:
        raise Exception("Don't know which minidump to check!")
    minidump_file = os.path.join(minidump_dir, minidump_files[0])
    md5sum = md5(minidump_file)
    while current < timeout:
        time.sleep(step)
        uploaded = kibana.crash_ids_from_timepoint(timepoint, product='demoapp', version='0.42')
        delta = { k : v for k, v in uploaded.items() if k not in dumps and md5sum == v }
        if len(delta) > 0:
            return delta
        else:
            current += step
    return None

