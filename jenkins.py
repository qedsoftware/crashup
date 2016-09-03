#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
"""Remote CI testing script. Test server username and domain name is provided
for convenience. This is generally heavy and if often you will need just to run
tests on your machine, in this case run one of 'build_*.py' scripts. This script
will read password from environmental variable CRASHUP_TEST_PASSWORD or else
prompt forpassword. Also, you can run it on different testing server by setting
the CRASHUP_TEST_HOSTSTRING to <your_username>@<your_server_dns_or_ip>. It is
assumed that the remote testing server has linux ubuntu operating system and
that your user can sudo without entering password. This script depends only on
fabric being installed on your machine.
"""
import os
import unittest
import sys
import subprocess
import getpass

import fabric
import fabric.contrib.files
from fabric.api import env, show, hide, run, put, local, cd, settings, prefix


def put_tar(local_name, remote_path):
    """Same as fabric.api.put, but sends tar archive with files instead of
    sending them one by one, which is _very_ slow. Also, assumes, that file
    named (local_name + ".tar") does not exist.
    """
    try:
        local('tar -c %s > %s.tar' % (local_name, local_name))
        put('%s.tar' % local_name, remote_path)
    finally:
        local('rm %s.tar' % local_name)
    with cd(remote_path):
        try:
            run('rm -rf %s' % local_name)
            run('tar -xf %s.tar' % local_name)
        finally:
            run('rm %s.tar' % local_name)


def install_clang_format():
    run('sudo apt-add-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main"')
    run('wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -')
    run("sudo apt-get update")
    run("sudo apt-get install -y --force-yes clang-format-3.7")


def remote_build(hoststring, password):
    env.host_string = hoststring
    env.password = password
    env.abort_on_prompts = True
    env.reject_unknown_hosts = False
    with show('exceptions'):
        try:
            run('clang-format-3.7 --version')
        except:
            install_clang_format()
        if not fabric.contrib.files.exists('~/desktop-crashup'):
            run('mkdir ~/desktop-crashup')
        things_to_put = [
            'demoapp', 'crashup', 'tests', 'cmake',
            'build_linux.py', 'CMakeLists.txt'
        ]

        for f in things_to_put:
            put_tar(f, '~/desktop-crashup/')
        with cd('~/desktop-crashup/'):
            if not fabric.contrib.files.exists('~/desktop-crashup/venv'):
                run('virtualenv venv')
            with prefix('source venv/bin/activate'):
                # verbose version in case of problems:
                # run('xvfb-run -e /dev/stdout -a python build_linux.py')
                # '-a' to try different display number if 99 is already taken
                run('xvfb-run -a python build_linux.py')


def main():
    hoststring = os.getenv('CRASHUP_TEST_HOSTSTRING')
    if hoststring is None:
        print("Please, provide CRASHUP_TEST_HOSTSTRING environment variable.")
        sys.exit(1)
    password = os.getenv('CRASHUP_TEST_PASSWORD')
    if password is None:
        password = getpass.getpass("%s password: " % DEFAULT_HOSTSTRING)
    remote_build(hoststring, password)


if __name__ == "__main__":
    main()

