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
import hashlib

import kibana

from .common import runapp, cleanup_minidumps, wait_until_minidump_is_uploading


class UploadMinidumpTests(unittest.TestCase):
    @unittest.skipUnless(sys.platform.startswith("linux"), "requires Linux")
    @cleanup_minidumps
    def test_upload_twice_on_linux(self):
        begin = kibana.now() - 3 * 1000
        dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
        with runapp() as app:
            app['segfaultButton'].click()
            time.sleep(0.5)
            self.assertEqual(len(os.listdir("minidumps")), 1)
        with runapp() as app:
            app['uploadButton'].click()
            minidumps = wait_until_minidump_is_uploading(begin, dumps)
            self.assertIsNotNone(minidumps)
        time.sleep(0.5)
        dumps.update(minidumps)
        with runapp() as app:
            app['uploadButton'].click()
            minidumps = wait_until_minidump_is_uploading(begin, dumps)
            self.assertIsNone(minidumps)
