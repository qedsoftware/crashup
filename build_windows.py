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
        import PIL
    except:
        do_call("pip", "install", "pillow")
    try:
        import pyautogui
    except:
        do_call("pip", "install", "pyautogui")


def build_app():
    # http://stackoverflow.com/questions/3618721/how-to-build-google-google-breakpad-for-windows
    shutil.rmtree("build", ignore_errors=True)
    os.mkdir("build")
    os.chdir("build")
    # the -DCMAKE_SYSTEM_VERSION=10.0 is _crucial_ when using VS 14 2015
    # to use new Windows 10.0 SDK instead of old 8.1 version that is
    # not compatibile with VS 14 2015. for more info, see:
    # http://stackoverflow.com/questions/31857315/how-can-i-use-cmake-to-generate-windows-10-universal-project
    # Win64 also is important here because we have only 64-bit version of Qt installed by now
    do_call(
        "cmake", "../demoapp",
        "-G", "Visual Studio 14 2015 Win64", "-DCMAKE_SYSTEM_VERSION=10.0",
        "-DCMAKE_PREFIX_PATH=C:\\Qt\\Qt5.6.0\\5.6\\msvc2015_64\\",
    )
    do_call("cmake", "--build", os.getcwd(), "--config", "Debug")
    os.chdir("..")


def copy_qt_dlls():
    dlls = '''
    Qt5Concurrent.dll Qt5Concurrentd.dll Qt5Core.dll Qt5Cored.dll Qt5Gui.dll
    Qt5Guid.dll Qt5Location.dll Qt5Locationd.dll Qt5Network.dll Qt5Networkd.dll
    Qt5Positioning.dll Qt5Positioningd.dll Qt5Qml.dll Qt5Qmld.dll Qt5Test.dll
    Qt5Testd.dll Qt5WebSockets.dll Qt5WebSocketsd.dll Qt5Widgets.dll
    Qt5Widgetsd.dll Qt5WinExtras.dll Qt5WinExtrasd.dll
    '''.split()
    for f in dlls:
        shutil.copy2(
            "C:\Qt\Qt5.6.0\\5.6\msvc2015_64\\bin\\" + f,
            "C:\Users\Administrator\Documents\desktop-crashup\\build\Debug\\"
        )


def run_tests():
    raise NotImplementedError
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
    print "CWD = " + os.getcwd()
    print "PATH = " + os.getenv("PATH")
    install_requirements()
    build_app()
    copy_qt_dlls()
    #run_tests()
