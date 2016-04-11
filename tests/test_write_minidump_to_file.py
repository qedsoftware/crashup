#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
import unittest
import sys
import time
from .widget_tracker import WidgetTracker
import shutil
import os


if sys.platform.startswith('linux'):
    APPNAME = "build/demoapp"
else:
    APPNAME = "build\Debug\demoapp.exe"


class WriteMinidumpTests(unittest.TestCase):
    def setUp(self):
        assert not os.path.exists("minidumps")

    def tearDown(self):
        shutil.rmtree("minidumps", ignore_errors=True)

    def test_segfault(self):
        with WidgetTracker(APPNAME, headless=True) as app:
            app['segfaultButton'].click()
            time.sleep(0.05)
            self.assertEqual(len(os.listdir("minidumps")), 1)

    def test_exception(self):
        with WidgetTracker(APPNAME, headless=True) as app:
            app['exceptionButton'].click()
            time.sleep(0.05)
            self.assertEqual(len(os.listdir("minidumps")), 1)
