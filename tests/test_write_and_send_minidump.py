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


def get_set_of_tracebacks(platform):
    import requests
    request = requests.get(
        "http://ec2-52-91-29-60.compute-1.amazonaws.com/api/list_minidumps/"
        )
    dumps = set()
    for x in request.json():
        if x['platform'] == platform:
            dumps.add(x['id'])
    return dumps


def get_crash_report_id(platform, crash_type, dumps):
    import requests
    request = requests.get(
        "http://ec2-52-91-29-60.compute-1.amazonaws.com/api/list_minidumps/"
        )
    for x in request.json():
        if x['platform'] == platform:
            if x['id'] in dumps:
                dumps.remove(x['id'])
            else:
                if crash_type in str(x['report_text']):
                    return x['id']
    return -1


class WriteMinidumpTests(unittest.TestCase):
    @cleanup_minidumps
    def test_segfault(self):
        if sys.platform.startswith('win'):
            dumps = get_set_of_tracebacks("windows")
            with runapp() as app:
                app['segfaultButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("crashdb/reports")), 1)
            id_to_remove = get_crash_report_id(
                "windows",
                "on_segfaultButton_clicked",
                dumps
            )
            self.assertTrue(id_to_remove >= 0)
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
            dumps = get_set_of_tracebacks("windows")
            with runapp() as app:
                app['exceptionButton'].click()
                time.sleep(0.5)
                self.assertEqual(len(os.listdir("crashdb/reports")), 1)
            id_to_remove = get_crash_report_id(
                "windows",
                "on_exceptionButton_clicked",
                dumps
            )
            self.assertTrue(id_to_remove >= 0)
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
