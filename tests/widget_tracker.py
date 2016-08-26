#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2013 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
import sys
import os
from subprocess import PIPE, Popen
from threading  import Thread
import re
import ast
import time
import PIL

import pyautogui

try:
    import pyvirtualdisplay
    import Xlib.display
except ImportError:
    pass

try:
    from Queue import Queue, Empty
except ImportError:
    from queue import Queue, Empty  # python 3.x


class NonBlockingStderrProcess(object):
    """Provide a (hopefully) cross-platform way to read from subprocess's
    stderr in a non-blocking way.
    """
    def __init__(self, args):
        """Open a process passing 'args' as first argument to subprocess.Popen()
        constructor. Spawn a new thread that will read this process's stderr
        line by line and put these lines into queue.
        """
        ON_POSIX = 'posix' in sys.builtin_module_names
        def enqueue_output(out, queue):
            for line in iter(out.readline, b''):
                queue.put(line)
            out.close()
        p = self.p = Popen(args, stderr=PIPE, bufsize=1, close_fds=ON_POSIX)
        q = self.q = Queue()
        t = self.t = Thread(target=enqueue_output, args=(p.stderr, q))
        t.daemon = True # thread dies with the program
        t.start()

    def fetch(self):
        """Return a list of all lines that has been written by the process to
        stderr by far.
        """
        res = []
        while True:
            try:
                line = self.q.get_nowait() # or q.get(timeout=.1)
            except Empty:
                break
            else:
                res.append(line)
        return res

    def terminate(self):
        """Terminate the running process."""
        self.p.terminate()


def take_screenshot(name="pyautogui"):
    filename = name + "_" + str(int(time.time())) + '.png'
    im = pyautogui.screenshot()
    cx, cy = pyautogui.position()
    col = (0, 255, 0)
    coords = [(x, 0) for x in range(-20, 20)] + [(0, y) for y in range(-20, 20)]
    def crop(x, y):
        w, h = pyautogui.size()
        x = max(x, 0)
        x = min(x, w-1)
        y = max(y, 0)
        y = min(y, h-1)
        return (x, y)
    for x, y in coords:
        im.putpixel(crop(cx+x, cy+y), col)
    im.save(filename)


class TrackedWidget(object):
    """Class that represents a widget-under-test.
    Keeps basic attributes of a QWidget:
        w->objectName() ==> tw.name
        w->geometry() ==> tw.left, tw.top, tw.right, tw.bottom
                        -- they are translated to global coordinates
        w->text() ==> tw.text  -- if applicable
        w->metaObject()->className() ==> tw.className
    Provides a way of triggering basic GUI events using the PyAutoGUI library:
        tw.click()
        tw.typewrite()
    """
    def __init__(self, name, tracker):
        self.name = name
        self.tracker = tracker

    def get_center(self):
        self.tracker.update()
        return (self.left + self.right)/2, (self.top + self.bottom)/2

    def hover_lefttop(self):
        self.tracker.update()
        pyautogui.moveTo(self.left, self.top, duration=0.3)

    def hover(self):
        pyautogui.moveTo(*self.get_center(), duration=0.3)

    def click(self):
        self.hover()
        pyautogui.click()

    def typewrite(self, txt):
        assert self.className == "QTextEdit"
        self.click()
        pyautogui.typewrite(txt, interval=0.1)


class WidgetTracker(object):
    """Class that represents Qt application under test. Has to be used as
    a context manager. In constructor you pass the path to application binary.
    If headless=True, application is launched in a virtual display so it is not
    visible. Application is started when __enter__ is called and terminated
    when __exit__ is called.
    """
    def __init__(self, appname, headless=False):
        # TODO add a mutex and spawn a thread that automatically updates the
        # parameters of tracked widgets instead of relying of a user to call
        # the update() method when necessary
        self.appname = appname
        self.proc = None
        self.widgets = {}
        self.headless = headless
        self.v_disp = None
        self.previous_pyautogui_display = None
        self.is_ready = False
        pyautogui.FAILSAFE = False

    def __enter__(self):
        if self.headless:
            self.v_disp = pyvirtualdisplay.Display(
                backend='xvfb', size=(800, 600)
            )
            self.v_disp.start()
            self.previous_pyautogui_display = pyautogui._pyautogui_x11._display
            pyautogui._pyautogui_x11._display = Xlib.display.Display(
                os.environ['DISPLAY']
            )
        self.proc = NonBlockingStderrProcess([self.appname, '--enable-widget-tracker'])
        try:
            # wait for application startup
            time_elapsed = 0.0
            time_allowed = 4.0  # TODO or something else, should be configurable
            while not self.is_ready:
                time.sleep(0.1)
                time_elapsed += 0.1
                self.update()
                if time_elapsed > time_allowed:
                    raise Exception(
                        "Application startup time exceeded allowed maximum of " +
                        str(time_allowed) + " seconds"
                    )
            print "Application startup time: " + str(time_elapsed)
        except:
            self.__exit__()  # context manager does not exit if enter fails
            raise
        return self

    def __exit__(self, *args):
        self.proc.terminate()
        if self.headless:
            pyautogui._pyautogui_x11._display = self.previous_pyautogui_display
            self.v_disp.stop()

    def _process_line(self, line):
        """Parse stderr line generated by WidgetTracker C++ part and update
        information about tracked widgets in self.widgets dictionary. Lines are
        in one of three forms:
            - "WidgetTracker: object.property = <value>"
            - "WidgetTracker: del object"
            - "WidgetTracker: READY"
        where <value> is a string literal (with quotes) or a number.
        Examples:
        WidgetTracker: myButton.left = 10
        WidgetTracker: myButton.className = "QPushButton"
        WidgetTracker: del myButton
        WidgetTracker: READY
        """
        assert line[0] == "WidgetTracker:"
        if len(line) == 4:
            # set property of a widget
            widget_name, prop_name = line[1].split('.')
            assert widget_name
            assert prop_name
            assert line[2] == '='
            prop = ast.literal_eval(line[3])
            assert isinstance(prop, int) or isinstance(prop, str)
            if widget_name not in self.widgets:
                self.widgets[widget_name] = TrackedWidget(widget_name, self)
            setattr(self.widgets[widget_name], prop_name, prop)
        elif len(line) == 3:
            # widget was hidden / destroyed, stop tracking it
            assert line[1] == 'del'
            self.widgets.pop(line[2], None)
        elif len(line) == 2 and line[1] == "READY":
            self.is_ready = True
        else:
            assert False

    def update(self):
        """Update information about tracked widgets in self.widgets dictionary
        by reading all available messages from stderr of application under test
        and passing the relevant ones to _process_line() function. Forward
        unrecognized messages to sys.stderr.
        """
        time.sleep(0.05)
        lines = self.proc.fetch()
        for line in lines:
            if "WidgetTracker:" not in line:
                sys.stderr.write(line)
                sys.stderr.flush()
            else:
                # regexp to split a string by whitespace, but treating
                # spaces inside quotes not as a separator
                l = re.findall(r'(?:[^\s,"]|"(?:\\.|[^"])*")+', line)
                self._process_line(l)

    def __getitem__(self, name):
        """Return TrackedWidget class corresponding to Qt widget with this name.
        (QObject's objectName() property)
        """
        return self.widgets[name]

    def __str__(self):
        """Convenience method to list names of all Qt widgets that are visible
        and have objectName() property set. Requires that in application under
        test WidgetTracker is enabled (app.installEventFilter(WidgetTracker)).
        """
        return str([w for w in self.widgets])


if __name__ == "__main__":
    # when you run this as a script, the python console appears and you can play
    # with the API (assuming that the target app is under the build/demoapp
    # path)
    import code
    with WidgetTracker("build/demoapp", headless=False) as app:
        print app
        code.interact(local=locals())
