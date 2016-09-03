#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
import os
import unittest
import sys
import subprocess
from subprocess import CalledProcessError
import shutil

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
TEST_DIRS = ['tests']


def do_call(*args):
  print("[{}]> {}".format(os.getcwd(), ' '.join(args)))
  subprocess.check_call(args)


def run_clang_format():
    cmd = (
        'shopt -s globstar && clang-format-3.7 -style=LLVM -output-replacements-xml '
        'crashup/**/*.cpp crashup/**/*.hpp demoapp/**/*.cpp demoapp/**/*.hpp '
        '| grep "<replacement "'
    )
    try:
        print "RUNNING CLANG-FORMAT CHECKER"
        do_call("bash", "-c", cmd)
    except CalledProcessError as e:
        if e.returncode != 1:
            raise
    else:
        raise Exception("CLANG-FORMAT FAILED!")


if __name__ == "__main__":
    if not sys.platform.startswith('linux'):
        print "Check other scripts for different platform."
    else:
        run_clang_format()
