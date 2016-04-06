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


DEFAULT_HOSTSTRING = "ubuntu@ec2-52-87-190-65.compute-1.amazonaws.com"


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


def install_cmake_repo():
    run("sudo apt-get install -y software-properties-common")
    run("sudo add-apt-repository -y ppa:george-edison55/cmake-3.x")
    run("sudo apt-get update")


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
        run('sudo apt-get -y build-dep python-imaging')
        pkgs = """
        libtiff5-dev libjpeg8-dev zlib1g-dev libfreetype6-dev liblcms2-dev
        libwebp-dev tcl8.6-dev tk8.6-dev python-tk tar xvfb cmake make
        python-pip libx11-dev python-virtualenv libjpeg-dev
        qt5-default qttools5-dev-tools cmake=3.2.2-2~ubuntu14.04.1~ppa1
        """
        try:
            run('sudo apt-get install -y ' + " ".join(pkgs.split()))
        except:
            install_cmake_repo()
            run('sudo apt-get install -y ' + " ".join(pkgs.split()))
        if not fabric.contrib.files.exists('~/desktop-crashup'):
            run('mkdir ~/desktop-crashup')
        things_to_put = [
            'demoapp', 'crashup', 'tests', 'build_linux.py',
        ]
        things_to_put_lazy = ['google-breakpad']
        for f in things_to_put:
            put_tar(f, '~/desktop-crashup/')
        for f in things_to_put_lazy:
            if not fabric.contrib.files.exists('~/desktop-crashup/' + f):
                put_tar(f, '~/desktop-crashup/')
        with cd('~/desktop-crashup/'):
            run('pwd')
            run('ls')
            if not fabric.contrib.files.exists('~/desktop-crashup/venv'):
                run('virtualenv venv')
            with prefix('source venv/bin/activate'):
                run('xvfb-run python build_linux.py')


def main():
    hoststring = os.getenv('CRASHUP_TEST_HOSTSTRING', DEFAULT_HOSTSTRING)
    password = os.getenv('CRASHUP_TEST_PASSWORD')
    if password is None:
        password = getpass.getpass("%s password: " % DEFAULT_HOSTSTRING)
    remote_build(hoststring, password)


if __name__ == "__main__":
    main()
