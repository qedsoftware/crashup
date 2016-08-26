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

from .common import runapp, cleanup_minidumps
if (not sys.platform.startswith('win')):
    from .common import check_if_minidump_upload_succeeded


def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest().lower()


def wait_until_minidump_is_uploading(timepoint, dumps, timeout=20., step=5.):
    current = 0.
    minidump_dir = os.path.join("crashdb", "reports")
    minidump_file = os.listdir(minidump_dir)[0]
    minidump_file = os.path.join(minidump_dir, minidump_file)
    md5sum = md5(minidump_file)
    while current < timeout:
        time.sleep(step)
        uploaded = kibana.crash_ids_from_timepoint(timepoint, product='demoapp', version='0.42')
        delta = { k : v for k, v in uploaded.items() if k not in dumps }
        if md5sum in delta.values():
            return delta
        else:
            current += step
    return None


class WriteMinidumpTests(unittest.TestCase):
    def check_minidump_uploaded_correctly_and_cleanup(self, timepoint, minidumps_before):
        minidumps = wait_until_minidump_is_uploading(timepoint, minidumps_before)
        self.assertIsNotNone(minidumps)

    @cleanup_minidumps
    def test_segfault(self):
        if sys.platform.startswith('win'):
            begin = kibana.now() - 3 * 1000
            dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
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
            begin = kibana.now()
            dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
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
    def test_segfault_in_library(self):
        if sys.platform.startswith('win'):
            begin = kibana.now()
            dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
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
    def test_exception_in_library(self):
        if sys.platform.startswith('win'):
            begin = kibana.now()
            dumps = kibana.crash_ids_from_timepoint(begin, product='demoapp', version='0.42')
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
            return
            with runapp() as app:
                app['uploadButton'].click()
                # 5 sec to wait for an upload to complete
                time.sleep(5.1)

                data = json.load(open("minidumps/.minidumps.json"))
                self.assertEqual(len(data),1)
                remote_filename = data[0]["remote_filename"]
                check_if_minidump_upload_succeeded(self,remote_filename)

