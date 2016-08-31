#!/usr/bin/env python
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------
"""For Windows testing, we depend on powershellserver
(http://www.powershellserver.com/download/) to be installed and running on
windows testing machine. Sftp and scp has to be enabled, and both should point
to "C:\Users\Administrator\Documents". Also, we treat this folder
as our working directory. We are not using WinRM because there is no secure
WinRM client running under linux (all send passwords in plaintext over http).

Powershellserver notes:
  - If you get this error message:
    "Cannot open certificate store: The parameter is incorrect. (0x80070057)",
    go to "Server Key" tab and generate new certificate/key.
  - Reinstalling in case the trial period expires:
    Delete 'C:\Program Files\nsoftware' and all the registry keys containing
    string 'nsoftware', then install powershellserver normally.
    Note: there is need to configure powershellserver SFTP root directory to be
    "C:\\" for file transfer to work properly (our script uses absolute paths,
    but root directory need to be set to "permit" those absolute paths)

Headless gui automation:
 - login through RDP (rdesktop) and open UltraVNC settings (uvnc_settings.exe)
 - click "install service" and "enable service" or something like this - run
    UltraVNC server as a service
 - set password (in ultravnc.ini there is an entry called "[ultravnc]passwd",
    but this is NOT the password! it's hash of it or something like that)
 - log out of RDP for UltraVNC to work properly (they are conflicting)
 - make sure that VNC ports are open (default are 5900 for desktop clients, 5800
    for browser-based java client)
 - log in through VNC with the password
 - click ctrl+alt+del and log in as Administrator
 - stop PowerShellServer if it is already running
 - start PowerShellServer but NOT as a windows service (as opposite to UltraVNC)
    this is because powershellserver must be launched from VNC session not from
    windows services because it has to have access to this VNC session's display
 - log out - close VNC client (the session with display will remain,
    and the tests will use it)
 - connect through ssh to powershellserver and launch tests from it
https://github.com/kybu/headless-selenium-for-win  <----  desktop_utils.exe
Probably far better way of doing it, however, I came across it when the former
approach was almost done. Also, this code is not tested well so it likely would
require similar amount of time to get it working. Just for the future, this
sounds like more elegant and less hacky way of doing gui automation on windows.
"""
import os
import unittest
import sys
import subprocess
import getpass
from StringIO import StringIO

import fabric
import fabric.contrib.files
from fabric.api import env, show, hide, run, put, local, cd, settings


DEFAULT_HOSTSTRING = "Administrator@107.23.3.175"

def install_dependencies():
    # install cmake, qt, python, pip, etc.
    # how to download file using powershell:
    # http://superuser.com/questions/25538/how-to-download-files-from-command-line-in-windows-like-wget-is-doing/755581
    # cmake: https://cmake.org/download/
    commands = ""
    commands += install_cmake()
    commands += install_python()
    #commands += install_vstudio2015()
    commands += install_vstudio2013()
    #commands += install_qt_for_vstudio2015()
    commands += install_qt_for_vstudio2013_x86_64()
    commands += install_qt_for_vstudio2013_i386()
    #commands += install_winsdk()
    commands += install_ultravnc()

    with hide('running'):
        run(commands)

def send_install_requirements():
    put_files(
        'qt-installer-noninteractive-vs2013-32.qs',
        'qt-installer-noninteractive-vs2013-64.qs',
        zipname='deploy_preinstall',
        remote_path='C:\Users\Administrator\Documents\desktop-crashup'
    )

def make_install_dependency_script(url, dest, name, path, options):
    """Generates commands that download installer and install the software
    Parameters
    ----------
    url : str
        The installer will be downloaded from this url.
    dest : str
        The installer will be saved to file specified with this parameter.
    name : str
        Dependency name
    path : str
        Path checked to determine if dependency is installed right now.
    options : str
        Additional options passed to the installer.
    """

    cmd = r'''
if(!(Test-Path "%s")) {
    $url = "%s"
    $output = "%s"
    $start_time = Get-Date
    Write-Output "Downloading %s..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    $time = (Get-Date).Subtract($start_time).Seconds
    Write-Output "Download completed after $time second(s)"
    Write-Output "Installing %s..."
    Start-Process "%s" %s -Wait
    Write-Output "%s installed"
} else {
    Write-Output "%s already installed"
}
    ''' % (path, url, dest, name, name, dest, options, name, name)
    return cmd

def install_cmake():
    cmake_url = "https://cmake.org/files/v3.5/cmake-3.5.1-win32-x86.msi"
    cmake_dest = "C:\Users\Administrator\Documents\cmake.msi"
    cmake_path = r'''C:\Program Files (x86)\CMake\bin'''
    cmake_name = "CMake"
    cmake_options = "/qn "
    return make_install_dependency_script(cmake_url, cmake_dest, cmake_name, cmake_path, cmake_options)

def install_python():
    python_url = "https://www.python.org/ftp/python/2.7.11/python-2.7.11.msi"
    python_dest = "C:\Users\Administrator\Documents\python.msi"
    python_path = "C:\Python27\python.exe"
    python_name = "Python"
    python_options = "/qn "
    return make_install_dependency_script(python_url, python_dest, python_name, python_path, python_options)

def install_qt_for_vstudio2015_x86_64():
    qt_url = "http://download.qt.io/official_releases/qt/5.6/5.6.0/qt-opensource-windows-x86-msvc2015_64-5.6.0.exe"
    qt_dest = r"C:\Users\Administrator\Documents\qtsetup_vs2015_64.exe"
    qt_path = r"C:\Qt\Qt5.6.0\5.6\msvc2015_64"
    qt_name = "Qt (for VS 14.0 2015, 64-bit)"
    qt_options = ""         # TODO: use qt-installer-...-32.qs, modify it (installation path!) and use
    return make_install_dependency_script(qt_url, qt_dest, qt_name, qt_path, qt_options)

def install_qt_for_vstudio2013_x86_64():
    qt_url = "http://download.qt.io/official_releases/qt/5.6/5.6.0/qt-opensource-windows-x86-msvc2013_64-5.6.0.exe"
    qt_dest = r"C:\Users\Administrator\Documents\qtsetup_vs2013_64.exe"
    qt_path = r"C:\Qt\Qt_5.6.0_vs2013_64bit"
    qt_name = "Qt (for VS 12.0 2013, 64-bit)"
    qt_options = "-ArgumentList @(\"--script\", \"C:\Users\Administrator\Documents\desktop-crashup\qt-installer-noninteractive-vs2013-64.qs\", \"--verbose\")"
    return make_install_dependency_script(qt_url, qt_dest, qt_name, qt_path, qt_options)

def install_qt_for_vstudio2013_i386():
    qt_url = "http://download.qt.io/official_releases/qt/5.6/5.6.0/qt-opensource-windows-x86-msvc2013-5.6.0.exe"
    qt_dest = r"C:\Users\Administrator\Documents\qtsetup_vs2013_32.exe"
    qt_path = r"C:\Qt\Qt_5.6.0_vs2013_32bit"
    qt_name = "Qt (for VS 12.0 2013, 32-bit)"
    qt_options = "-ArgumentList @(\"--script\", \"C:\Users\Administrator\Documents\desktop-crashup\qt-installer-noninteractive-vs2013-32.qs\", \"--verbose\")"
    return make_install_dependency_script(qt_url, qt_dest, qt_name, qt_path, qt_options)

def install_winsdk():
    winsdk_url = "http://download.microsoft.com/download/6/A/2/6A2ECE81-C934-4E47-91CC-52DA00A65345/sdksetup.exe"
    winsdk_dest = "C:\Users\Administrator\Documents\sdksetup10.exe"
    winsdk_path = "C:\Program Files (x86)\Windows Kits\\10"
    winsdk_name = "Windows SDK 10"
    winsdk_options = "/q "
    return make_install_dependency_script(winsdk_url, winsdk_dest, winsdk_name, winsdk_path, winsdk_options)

def install_ultravnc():
    ultravnc_url = "http://www.uvnc.com/component/jdownloads/finish/4-setup/291-ultravnc-1210-x64-setup/0.html"
    ultravnc_dest = "C:\Users\Administrator\Documents\ultravnc_setup.exe"
    ultravnc_path = "C:\Program Files\UltraVNC"
    ultravnc_name = "UltraVNC"
    ultravnc_options = "-ArgumentList '/DIR=\"C:\Program Files\UltraVNC\"',\"/SILENT\",'/TYPE=\"UltraVNC Server Only\"',\"/LOG\" "
    return make_install_dependency_script(ultravnc_url, ultravnc_dest, ultravnc_name, ultravnc_path, ultravnc_options)

def install_vstudio2015():
    vstudio_url = "http://download.microsoft.com/download/D/2/3/D23F4D0F-BA2D-4600-8725-6CCECEA05196/vs_community_ENU.exe"
    vstudio_dest = "C:\Users\Administrator\Documents\\vs_community.exe"
    vstudio_path = "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
    vstudio_name = ""
    cmd = r'''
if(!(Test-Path "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC")) {
    $url = "%s"
    $output = "%s"
    Write-Output "Downloading Visual Studio 2015..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Write-Output "Download completed"
    Write-Output "You have to install Visual Studio 2015 (including Visual C++), installation file: %s"
    Exit 1
} else {
    Write-Output "Visual Studio 2015 already installed"
}
    ''' % (vstudio_url, vstudio_dest, vstudio_dest)
    return cmd

def install_vstudio2013():
    vstudio2013_url = " http://download.microsoft.com/download/A/A/D/AAD1AA11-FF9A-4B3C-8601-054E89260B78/vs_community.exe"
    vstudio2013_dest = "C:\Users\Administrator\Documents\\vs2013_community.exe"
    cmd = r'''
if(!(Test-Path "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC")) {
    $url = "%s"
    $output = "%s"
    Write-Output "Downloading Visual Studio 2013..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Write-Output "Download completed"
    Write-Output "You have to install Visual Studio 2013 (including Visual C++), installation file: %s"
    Exit 1
} else {
    Write-Output "Visual Studio 2013 already installed"
}
    ''' % (vstudio2013_url, vstudio2013_dest, vstudio2013_dest)
    return cmd



def adjust_env():
    """Set the $Env:PATH (%Path% environment variable) so that all installed
    binaries are visible. If you have any problems with them (or want to undo
    adding something to $Env:PATH), read the below explanation to understand
    how they work with PSServer*.
    * - PSServer = PowerShellServer
    Environment variables:
        It turned out that if PSServer runs not as a windows service (we
        haven't tested running as windows service) then the $Env is presisted
        between logins. Even if you click "restart" in PSServer GUI, it still
        retains it. The way of resetting them that works is to kill the PSServer
        background process from the windows task manager and then start the
        server. This way you can reset environment variables in PSServer.
    Useful link explaining how env vars on windows work:
    http://windowsitpro.com/powershell/take-charge-environment-variables-powershell
    """
    cmd = r'''
$paths = "C:\Python27;C:\Python27\Scripts;C:\Program Files (x86)\CMake\bin"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain CMake and Python executables..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
    Write-Output "CMake and Python already present in Env:Path"
}
    '''
    cmd += r'''
$paths = "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain Visual C++ executables..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
    Write-Output "Visual C++ executables already present in Env:Path"
}
    '''
    cmd += r'''
$paths = "C:\Program Files (x86)\MSBuild\12.0\Bin"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain MSBuild executables..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
    Write-Output "MSBuild executables already present in Env:Path"
}
    '''
    cmd += r'''
$paths = "C:\Program Files\UltraVNC"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain UltraVNC executables..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
    Write-Output "UltraVNC executables already present in Env:Path"
}
    '''
    cmd += r'''
$paths = "C:\Users\Administrator\Documents\desktop-crashup\google-crashpad\depot_tools"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain chromium depot_tools..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
    Write-Output "depot_tools already present in Env:Path"
}
    '''
    with hide('running'):
        run(cmd)


def remote_file_exists(path):
    cmd = '''
    if(Test-Path "%s") {
        Write-Output "Yes"
    } else {
        Write-Output "No"
    }
    ''' % path
    with hide('running', 'output'):
        out = run(cmd).stdout
    out = str(out.strip())
    if "Yes" in out:
        return True
    elif "No" in out:
        return False
    else:
        # if you change above "in" to "==", a strange bug appears:
        # out is a corrupted string, it is printed correctly as 'No', but
        # len(out) == 32 and if you iterate through it, you will get about 30
        # bytes of garbage at the beginning and the 2 letters 'N' and 'o' at the
        # end.
        # THIS BEHAVIOR WAS OBSERVED WHEN USING FABRIC PSEUDOTERMINAL SETTING
        # (WHICH IS DEFAULT ONE), NOW WE SWITCHED IT OFF, MAYBE THIS HELPED
        res = ""
        cnt = 0
        for i in out:
            print '"' + i + '"'
            cnt += 1
            res += i
        print res
        print cnt
        print ('No' in out)
        print (out in 'No')
        print len(out)
        print out[-20:]
        print out[-10:]
        print (out == 'No')
        print ('No' == out)
        print "out = '" + out + "'"
        print type(out)
        raise Exception("Something unexpected happened!")


def put_files(*files, **kwargs):
    zipname = kwargs.pop('zipname', None)
    remote_path = kwargs.pop('remote_path', None)
    assert zipname
    assert remote_path
    assert not kwargs
    try:
        local('zip -q -r %s.zip %s' % (zipname, " ".join(files)))
        put('%s.zip' % zipname, remote_path)
    finally:
        local('rm %s.zip' % zipname)
    cmd = '''
Add-Type -AssemblyName System.IO.Compression.FileSystem
function Unzip
{
param([string]$zipfile, [string]$outpath)
Write-Output "Extracting $zipfile"
Write-Output "to $outpath"
[System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}
    ''' + ("\n".join(['''
if(Test-Path "%s\%s") {
    Write-Output "Removing old version of %s"
    Remove-Item %s\%s -Force -Recurse
}
    ''' % (remote_path, f, f, remote_path, f) for f in files])) + ('''
Unzip "%s\%s.zip" "%s"
    ''' % (remote_path, zipname, remote_path)) + ('''
Write-Output "Removing %s.zip"
rm %s\%s.zip
    ''' % (zipname, remote_path, zipname))
    with hide('running'):
        run(cmd)


def send_files():
    with hide('output'):
        run('md -Force C:\Users\Administrator\Documents\desktop-crashup')
    print INDENT + "Sending source tree to remote host..."
    put_files(
        'demoapp', 'crashup', 'tests', 'scripts',
        'build_linux.py', 'build_windows.py',
        zipname='deploy',
        remote_path='C:\Users\Administrator\Documents\desktop-crashup'
    )

    print INDENT + "Sending gyp config file to remote host..."
    put_files(
        'common.gypi', zipname='common.gypi',
        remote_path='C:\Users\Administrator\Documents\desktop-crashup\google-breakpad\src\\build'
    )


def run_winvnc_service():
    # this function ended up not being used as everything needs to be done
    # manually nevertheless
    cmd = '''
    Write-Output "Running UltraVNC service..."
    Start-Process "C:\Program Files\UltraVNC\winvnc.exe" -ArgumentList '-service' # don't -Wait
    '''
    with hide('running'):
        run(cmd)


def remote_build(hoststring, password):
    env.host_string = hoststring
    env.password = password
    env.abort_on_prompts = True
    env.reject_unknown_hosts = False
    env.use_shell = False  # we don't have bash on windows
    env.always_use_pty = False  # to prevent truncating lines by the server
    with show('exceptions'):
        send_install_requirements()
        install_dependencies()
        send_files()
        adjust_env()
        cmd = '''
        cd desktop-crashup
        C:\Python27\python.exe build_windows.py
        '''
        run(cmd)


def main():
    hoststring = os.getenv('CRASHUP_TEST_WINDOWS_HOSTSTRING', DEFAULT_HOSTSTRING)
    global INDENT
    INDENT = "[" + (" " * len(hoststring)) + "] >>>| "
    password = os.getenv('CRASHUP_TEST_WINDOWS_PASSWORD')
    if password is None:
        password = getpass.getpass("%s password: " % DEFAULT_HOSTSTRING)
    remote_build(hoststring, password)


if __name__ == "__main__":
    main()
