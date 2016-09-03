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
from time import sleep


THIS_DIR = os.path.dirname(os.path.abspath(__file__))
TEST_DIRS = ['tests']


def do_call(*args):
  print("[{}]> {}".format(os.getcwd(), ' '.join(args)))
  subprocess.check_call(args)


def install_requirements():
    try:
        import PIL
    except:
        do_call("pip", "install", "pillow")
    try:
        import pyautogui
    except:
        do_call("pip", "install", "pyautogui")
    try:
        import requests
    except:
        do_call("pip", "install", "requests")


def build_app(source_root, build_path, generator):
    # http://stackoverflow.com/questions/3618721/how-to-build-google-google-breakpad-for-windows
    if os.path.exists(build_path):
        shutil.rmtree(build_path)
    os.mkdir(build_path)
    os.chdir(build_path)
    # the -DCMAKE_SYSTEM_VERSION=10.0 is _crucial_ when using VS 14 2015
    # to use new Windows 10.0 SDK instead of old 8.1 version that is
    # not compatibile with VS 14 2015. for more info, see:
    # http://stackoverflow.com/questions/31857315/how-can-i-use-cmake-to-generate-windows-10-universal-project
    # Win64 also is important here because we have only 64-bit version of Qt installed by now
    do_call(
        "cmake", source_root,
        "-G", generator, #"-DCMAKE_SYSTEM_VERSION=10.0",
        "-DSOCORRO_UPLOAD_URL=%s" % SOCORRO_UPLOAD_URL,
    )
    do_call("cmake", "--build", os.getcwd(), "--config", "Debug")
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
    print "Building in workspace: " + os.getcwd()
    install_requirements()

    global SOCORRO_UPLOAD_URL
    global KIBANA_URL

    SOCORRO_UPLOAD_URL = sys.argv[1]
    KIBANA_URL = sys.argv[2]

    with open('tests/private.py', 'w') as f:
      f.write('''KIBANA_URL="%s"
''' % (KIBANA_URL)
      )

    # 64-bit
    source_root = "C:\\Users\\Administrator\\Documents\\desktop-crashup"
    build_path = "C:\\Users\\Administrator\\Documents\\desktop-crashup\\build"
    generator = "Visual Studio 12 2013 Win64"
    build_app(source_root, build_path, generator)
    run_tests()

    # 32-bit
    source_root = "C:\\Users\\Administrator\\Documents\\desktop-crashup"
    build_path = "C:\\Users\\Administrator\\Documents\\desktop-crashup\\build"
    generator = "Visual Studio 12 2013"     # Win32
    build_app(source_root, build_path, generator)
    run_tests()
