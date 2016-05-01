#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
import unittest
import time
import shutil
import os
import sys

from .common import runapp, cleanup_minidumps


class WriteMinidumpTests(unittest.TestCase):
    @cleanup_minidumps
    def test_segfault(self):
        # FIXME FIXME FIXME
        # turned off on windows until we fix our google breakpad installation
        if sys.platform.startswith('win'):
            return
        with runapp() as app:
            app['segfaultButton'].click()
            time.sleep(0.05)
            self.assertEqual(len(os.listdir("minidumps")), 1)

    @cleanup_minidumps
    def test_exception(self):
        with runapp() as app:
            app['exceptionButton'].click()
            time.sleep(0.05)
            self.assertEqual(len(os.listdir("minidumps")), 1)
