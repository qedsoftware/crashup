import os
import time
import hashlib
import unittest
from .widget_tracker import WidgetTracker

class DownloaderTest(unittest.TestCase):
    def testFileDownload(self):
        with WidgetTracker("build/demoapp", headless=True) as app:
            app['url'].typewrite("https://www.google.pl/images/branding/"
                                 "product/ico/googleg_lodp.ico")
            app['downloadDirectory'].typewrite("build")
            app['downloadButton'].click()

            t = 0
            while not os.path.exists("build/googleg_lodp.ico") and t < 15:
                t += 0.25
                time.sleep(0.25)

            self.assertTrue(os.path.exists("build/googleg_lodp.ico"))
            index = open("build/googleg_lodp.ico", "r")
            content = index.read()
            self.assertEqual(hashlib.md5(content).hexdigest(),
                        'f3418a443e7d841097c714d69ec4bcb8')
                
            
