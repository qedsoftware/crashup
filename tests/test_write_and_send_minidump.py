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
import json

import kibana

from .common import runapp, cleanup_minidumps, wait_until_minidump_is_uploading


class WriteMinidumpTests(unittest.TestCase):
    def check_minidump_uploaded_correctly_and_cleanup(self, timepoint, minidumps_before):
        minidumps = wait_until_minidump_is_uploading(timepoint, minidumps_before)
        self.assertIsNotNone(minidumps)

    @cleanup_minidumps
    def test_segfault(self):
        begin = kibana.now() - 3 * 1000
        dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
        if sys.platform.startswith('win'):
            with runapp() as app:
                app['segfaultButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("crashdb/reports")), 1)
            self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)
        else:
            with runapp() as app:
                app['segfaultButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("minidumps")), 1)
            with runapp() as app:
                app['uploadButton'].click()
                self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)

    @cleanup_minidumps
    def test_exception(self):
        begin = kibana.now()
        dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
        if sys.platform.startswith('win'):
            with runapp() as app:
                app['exceptionButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("crashdb/reports")), 1)
            self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)
        else:
            with runapp() as app:
                app['exceptionButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("minidumps")), 1)
            with runapp() as app:
                app['uploadButton'].click()
                self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)

    @cleanup_minidumps
    def test_segfault_in_library(self):
        begin = kibana.now()
        dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
        if sys.platform.startswith('win'):
            with runapp() as app:
                app['useMakeSegv'].click()
                app['segfaultButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("crashdb/reports")), 1)
            self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)
        else:
            with runapp() as app:
                app['useMakeSegv'].click()
                app['segfaultButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("minidumps")), 1)
            with runapp() as app:
                app['uploadButton'].click()
                self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)

    @cleanup_minidumps
    def test_exception_in_library(self):
        begin = kibana.now()
        dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
        if sys.platform.startswith('win'):
            with runapp() as app:
                app['useMakeSegv'].click()
                app['exceptionButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("crashdb/reports")), 1)
            self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)
        else:
            with runapp() as app:
                app['useMakeSegv'].click()
                app['exceptionButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("minidumps")), 1)
            with runapp() as app:
                app['uploadButton'].click()
                self.check_minidump_uploaded_correctly_and_cleanup(begin, dumps)
