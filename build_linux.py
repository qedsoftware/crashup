#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
import os
import unittest
import sys
import subprocess
from subprocess import CalledProcessError
import shutil

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
TEST_DIRS = ['tests']


def do_call(*args):
  print("[{}]> {}".format(os.getcwd(), ' '.join(args)))
  subprocess.check_call(args)


def install_requirements():
    try:
        import Xlib
    except:
        do_call("pip", "install", "http://downloads.sourceforge.net/project/python-xlib/python-xlib/0.15rc1/python-xlib-0.15rc1.tar.gz?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fpython-xlib%2Ffiles%2Fpython-xlib%2F0.15rc1%2F&ts=1459604321&use_mirror=tenet")  # NOQA
    try:
        import PIL
    except:
        do_call("pip", "install", "pillow")
    try:
        import pyvirtualdisplay
    except:
        do_call("pip", "install", "pyvirtualdisplay")
    try:
        import pyautogui
    except:
        do_call("pip", "install", "pyautogui")


def run_clang_format():
    cmd = (
        'shopt -s globstar && clang-format-3.7 -style=LLVM -output-replacements-xml '
        'crashup/**/*.cpp crashup/**/*.hpp demoapp/**/*.cpp demoapp/**/*.hpp '
        '| grep "<replacement "'
    )
    try:
        print "RUNNING CLANG-FORMAT CHECKER"
        do_call("bash", "-c", cmd)
    except CalledProcessError as e:
        if e.returncode != 1:
            raise
    else:
        raise Exception("CLANG-FORMAT FAILED!")


def build_app():
    shutil.rmtree("build", ignore_errors=True)
    os.mkdir("build")
    os.chdir("build")
    do_call("cmake", "../demoapp")
    do_call("make")
    os.chdir("..")


def run_tests():
    loader = unittest.TestLoader()
    tests = unittest.TestSuite([
        loader.discover(
            os.path.join(THIS_DIR, d),
            top_level_dir=THIS_DIR
        ) for d in TEST_DIRS
    ])
    result = unittest.runner.TextTestRunner().run(tests)
    if not result.wasSuccessful():
        raise Exception('Test failed')


if __name__ == "__main__":
    if not sys.platform.startswith('linux'):
        print "TODO: support systems other than linux"
    else:
        install_requirements()
        build_app()
        run_tests()
        run_clang_format()
