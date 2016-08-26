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



def check_if_minidump_upload_succeeded(self, remote_filename):
    hoststring = os.getenv('CRASHUP_TEST_HOSTSTRING', DEFAULT_HOSTSTRING)
    password = os.getenv('CRASHUP_TEST_PASSWORD', DEFAULT_PASSWORD)

    env.host_string = hoststring
    env.password = password
    env.abort_on_prompts = True
    env.reject_unknown_hosts = False

    date_foldername = time.strftime("%Y%m%d")
    with cd("/home/socorro/crashes/"):
        res = remote_filename.split("/")
        self.assertTrue(exists(date_foldername+"/name/"+remote_filename+".dump", use_sudo=True))
        self.assertTrue(exists(date_foldername+"/name/"+remote_filename+".json", use_sudo=True))
        # file removing, with their corresponding directory if it does not contain anything else
        sudo("rm -f "+date_foldername+"/name/"+remote_filename+".dump")
        self.assertFalse(exists(date_foldername+"/name/"+remote_filename+".dump", use_sudo=True))
        sudo("rm -f "+date_foldername+"/name/"+remote_filename+".json")
        self.assertFalse(exists(date_foldername+"/name/"+remote_filename+".json", use_sudo=True))

        # checking for regular files or directories to avoid finding occasional broken symlinks
        if (len(run("find ./"+date_foldername+"/name/"+res[0]+"/"+res[1]+"/ -type f || -type d")) == 0):
            sudo("rm -rf "+date_foldername+"/name/"+res[0]+"/"+res[1]+"/")
        if (len(run("find ./"+date_foldername+"/name/"+res[0]+"/ -type f || -type d")) == 0):
            sudo("rm -rf "+date_foldername+"/name/"+res[0]+"/")
        if (len(run("find ./"+date_foldername+"/name/ -type f || -type d")) == 0):
            sudo("rm -rf "+date_foldername+"/")
