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


DEFAULT_HOSTSTRING = "Administrator@54.84.41.90"


def install_dependencies():
    # install cmake, qt, python, pip, etc.
    # how to download file using powershell:
    # http://superuser.com/questions/25538/how-to-download-files-from-command-line-in-windows-like-wget-is-doing/755581
    # cmake: https://cmake.org/download/
    commands = ""
    cmake_url = "https://cmake.org/files/v3.5/cmake-3.5.1-win32-x86.msi"
    cmake_dest = "C:\Users\Administrator\Documents\cmake.msi"
    install_cmake = r'''
if(!(Test-Path "C:\Program Files (x86)\CMake\bin")) {
    $url = "%s"
    $output = "%s"
    $start_time = Get-Date
    Write-Output "Downloading CMake..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    $time = (Get-Date).Subtract($start_time).Seconds
    Write-Output "Download completed after $time second(s)"
    Write-Output "Installing CMake..."
    Start-Process "%s" /qn -Wait
    Write-Output "CMake installed"
} else {
    Write-Output "CMake already installed"
}
    ''' % (cmake_url, cmake_dest, cmake_dest)
    commands += install_cmake

    python_url = "https://www.python.org/ftp/python/2.7.11/python-2.7.11.msi"
    python_dest = "C:\Users\Administrator\Documents\python.msi"
    install_python = r'''
if(!(Test-Path "C:\Python27\python.exe")) {
    $url = "%s"
    $output = "%s"
    $start_time = Get-Date
    Write-Output "Downloading Python..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    $time = (Get-Date).Subtract($start_time).Seconds
    Write-Output "Download completed after $time second(s)"
    Write-Output "Installing Python..."
    Start-Process "%s" /qn -Wait
    Write-Output "Python installed"
} else {
    Write-Output "Python already installed"
}
    ''' % (python_url, python_dest, python_dest)
    commands += install_python

    # FIXME FIXME FIXME: this is done now that it will first time install the
    # visual studio and then hang, and second time it will succeed saying
    # that VS is already installed. FIX: TODO: this should launch the installer
    # in the background and poll for the required folder to exist, then proceed
    # to next steps.
    # http://timsneath.com/visual-studio-2015-installation-options/
    vstudio_url = "http://download.microsoft.com/download/D/2/3/D23F4D0F-BA2D-4600-8725-6CCECEA05196/vs_community_ENU.exe"
    vstudio_dest = "vs_community.exe"
    install_vstudio = r'''
if(!(Test-Path "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC")) {
    $url = "%s"
    $output = "C:\Users\Administrator\Documents\%s"
    Write-Output "Downloading Visual Studio..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Write-Output "Download completed"
    Write-Output "Installing Visual Studio... (this may take a few hours)"
    $start_time = Get-Date
    Start-Process "%s" -ArgumentList "/quiet","/installselectableitems","NativeLanguageSupport_Group;CommonTools_Group" -Wait
    $time = (Get-Date).Subtract($start_time).Seconds
    Write-Output "Visual Studio installed after $time second(s)"
} else {
    Write-Output "Visual Studio already installed"
}
    ''' % (vstudio_url, vstudio_dest, vstudio_dest)
    commands += install_vstudio

    # FIXME FIXME FIXME: add unattended Qt install script according to these
    # instructions: https://doc.qt.io/qtinstallerframework/noninteractive.html
    qt_url = "http://download.qt.io/official_releases/qt/5.6/5.6.0/qt-opensource-windows-x86-msvc2015_64-5.6.0.exe"
    qt_dest = "C:\Users\Administrator\Documents\qtsetup.exe"
    install_qt = r'''
if(!(Test-Path "C:\Qt\Qt5.6.0")) {
    $url = "%s"
    $output = "%s"
    $start_time = Get-Date
    Write-Output "Downloading Qt..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    $time = (Get-Date).Subtract($start_time).Seconds
    Write-Output "Download completed after $time second(s)"
    Write-Output "Installing Qt..."
    Start-Process "%s" -Wait
    Write-Output "Qt installed"
} else {
    Write-Output "Qt already installed"
}
    ''' % (qt_url, qt_dest, qt_dest)
    commands += install_qt

    winsdk_url = "http://download.microsoft.com/download/6/A/2/6A2ECE81-C934-4E47-91CC-52DA00A65345/sdksetup.exe"
    winsdk_dest = "C:\Users\Administrator\Documents\sdksetup10.exe"
    install_winsdk = r'''
if(!(Test-Path "C:\Program Files (x86)\Windows Kits\10")) {
    $url = "%s"
    $output = "%s"
    $start_time = Get-Date
    Write-Output "Downloading Windows SDK 10..."
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    $time = (Get-Date).Subtract($start_time).Seconds
    Write-Output "Download completed after $time second(s)"
    Write-Output "Installing Windows SDK 10..."
    Start-Process "%s" /q -Wait
    Write-Output "Windows SDK 10 installed"
} else {
    Write-Output "Windows SDK 10 already installed"
}
    ''' % (winsdk_url, winsdk_dest, winsdk_dest)
    commands += install_winsdk

    with hide('running'):
        run(commands)


def adjust_env():
    cmd = r'''
$paths = "C:\Python27;C:\Python27\Scripts;C:\Program Files (x86)\CMake\bin"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain CMake and Python executables..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
    "CMake and Python already present in Env:Path"
}
    '''
    cmd += r'''
$paths = "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin;C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain Visual C++ executables..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
"Visual C++ executables already present in Env:Path"
}
'''
    cmd += r'''
$paths = "C:\Program Files (x86)\MSBuild\14.0\Bin"
if(!($Env:Path -like ("*" + $paths + "*"))) {
    Write-Output "Adjusting Env:Path to contain MSBuild executables..."
    $Env:Path = $Env:Path + ";" + $paths
} else {
    Write-Output "MSBuild executables already present in Env:Path"
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


def compile_google_breakpad():
    pref = "C:\Users\Administrator\Documents\desktop-crashup\google-breakpad"
    cmd = r'''
    Write-Output "Compiling google breakpad..."
    $Env:GYP_MSVS_VERSION = "2013"
    %s\src\tools\gyp\gyp.bat --no-circular-check %s\src\client\windows\breakpad_client.gyp
    msbuild %s\src\client\windows\breakpad_client.sln /p:PlatformToolset=v140 /p:VisualStudioVersion=14.0 /ToolsVersion:14.0 /p:Platform="x64"
    ''' % (pref, pref, pref)
    with hide('running'):
        run(cmd)


'''
  processor_bits.lib(minidump.obj) : error LNK2019: unresolved external symbol
"bool __cdecl google_breakpad::ParseProcMaps(class std::basic_string<char,struc
t std::char_traits<char>,class std::allocator<char> > const &,class std::vector
<struct google_breakpad::MappedMemoryRegion,class std::allocator<struct google_
breakpad::MappedMemoryRegion> > *)" (?ParseProcMaps@google_breakpad@@YA_NABV?$b
asic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PAV?$vector@UMappedMe
moryRegion@google_breakpad@@V?$allocator@UMappedMemoryRegion@google_breakpad@@@
std@@@3@@Z) referenced in function "private: virtual bool __thiscall google_bre
akpad::MinidumpLinuxMapsList::Read(unsigned int)" (?Read@MinidumpLinuxMapsList@
google_breakpad@@EAE_NI@Z) [C:\Users\Administrator\Documents\desktop-crashup\go 

https://bugs.chromium.org/p/google-breakpad/issues/detail?id=669
'''


def send_files():
    with hide('output'):
        run('md -Force C:\Users\Administrator\Documents\desktop-crashup')
    print INDENT + "Sending source tree to remote host..."
    put_files(
        'demoapp', 'crashup', 'tests', 'build_linux.py', 'build_windows.py',
        zipname='deploy',
        remote_path='C:\Users\Administrator\Documents\desktop-crashup'
    )
    if not remote_file_exists('C:\Users\Administrator\Documents\desktop-crashup\google-breakpad'):
        print INDENT + "Sending google-breakpad to remote host..."
        put_files(
            'google-breakpad', zipname='google-breakpad',remote_path='C:\Users\Administrator\Documents\desktop-crashup'
        )
    else:
        print INDENT + "google-breakpad already present on remote host"
    
    print INDENT + "Sending gyp config file to remote host..."
    put_files(
        'common.gypi', zipname='common.gypi',
        remote_path='C:\Users\Administrator\Documents\desktop-crashup\google-breakpad\src\\build'
    )


def remote_build(hoststring, password):
    env.host_string = hoststring
    env.password = password
    env.abort_on_prompts = True
    env.reject_unknown_hosts = False
    env.use_shell = False  # we don't have bash on windows
    env.always_use_pty = False  # to prevent truncating lines by the server
    with show('exceptions'):
        install_dependencies()
        adjust_env()
        send_files()
        # TODO compile only when not present!
        compile_google_breakpad()
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
