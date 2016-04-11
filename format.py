#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
import os
import subprocess

def do_call(*args):
  print("[{}]> {}".format(os.getcwd(), ' '.join(args)))
  subprocess.check_call(args)

cmd = (
        'shopt -s globstar && clang-format-3.7 -style=LLVM -i '
        'crashup/**/*.cpp crashup/**/*.hpp demoapp/**/*.cpp demoapp/**/*.hpp '
)

print "FORMATTING ALL FILES USING CLANG-FORMAT"
do_call("bash", "-c", cmd)
