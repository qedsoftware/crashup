import os
import sys
import time
import hashlib
import unittest

from .common import runapp


class DownloaderTest(unittest.TestCase):
    def test_file_download(self):
        with runapp() as app:
            # TODO: ship OpenSSL dll-s with application on windows
            # now using http instead of https
            if not sys.platform.startswith('linux'):
                app['url'].typewrite("http://www.google.pl/images/branding/"
                                     "product/ico/googleg_lodp.ico")
            else:
                app['url'].typewrite("https://www.google.pl/images/branding/"
                                     "product/ico/googleg_lodp.ico")
            app['downloadDirectory'].typewrite("build")
            app['downloadButton'].click()

            d_path = os.path.join("build", "googleg_lodp.ico")

            t = 0
            while not os.path.exists(d_path) and t < 15:
                t += 0.25
                time.sleep(0.25)

            self.assertTrue(os.path.exists(d_path))
            index = open(d_path, "r")
            content = index.read()
            self.assertEqual(hashlib.md5(content).hexdigest(),
                        'f3418a443e7d841097c714d69ec4bcb8')
