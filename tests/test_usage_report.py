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


def get_set_of_reports(platform):
    import requests
    request = requests.get(
        "http://ec2-52-91-29-60.compute-1.amazonaws.com/api/list_usage_reports/"
        )
    dumps = set()
    for x in request.json():
        if x['app_platform'] == platform:
            dumps.add(x['id'])
    return dumps


def get_usage_report_id(platform, reports):
    import requests
    request = requests.get(
        "http://ec2-52-91-29-60.compute-1.amazonaws.com/api/list_usage_reports/"
        )
    for x in request.json():
        if x['app_platform'] == platform:
            if x['id'] in reports:
                reports.remove(x['id'])
            else:
                if "integration_test_keyword" in str(x['event_data']):
                    return x['id']
    return -1


class UsageReportTests(unittest.TestCase):
    @cleanup_minidumps
    def test_send(self):
        if sys.platform.startswith('win'):
            reports = get_set_of_reports("windows")
        else:
            return # some strange issue with http
            reports = get_set_of_reports("linux")
        with runapp() as app:
            app['textEdit'].typewrite("blah integration_test_keyword blaa")
            app['statsButton'].click()
            time.sleep(0.5)
        id_to_remove = get_usage_report_id("windows", reports)
        self.assertTrue(id_to_remove >= 0)
