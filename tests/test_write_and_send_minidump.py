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

from .common import runapp, cleanup_minidumps
if (not sys.platform.startswith('win')):
    from .common import check_if_minidump_upload_succeeded

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
        return
        with runapp() as app:
            app['uploadButton'].click()
            # 5 sec to wait for an upload to complete
            time.sleep(5.1)
    
            data = json.load(open("minidumps/.minidumps.json"))
            self.assertEqual(len(data),1)
            remote_filename = data[0]["remote_filename"]
            check_if_minidump_upload_succeeded(self,remote_filename)

    @cleanup_minidumps
    def test_exception(self):
        if sys.platform.startswith('win'):
            return
        with runapp() as app:
            app['exceptionButton'].click()
            time.sleep(0.05)
            self.assertEqual(len(os.listdir("minidumps")), 1)
        return
        with runapp() as app:
            app['uploadButton'].click()
            # 5 sec to wait for an upload to complete
            time.sleep(5.1)

            data = json.load(open("minidumps/.minidumps.json"))
            self.assertEqual(len(data),1)
            remote_filename = data[0]["remote_filename"]
            check_if_minidump_upload_succeeded(self,remote_filename)
