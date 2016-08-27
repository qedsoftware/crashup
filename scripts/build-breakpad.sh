#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# Copyright (c) 2016 by Quantitative Engineering Design, Inc.
# ALL RIGHTS RESERVED.
# QED | Contact: William Wu <w@qed.ai>
# -----------------------------------------------------------------------------

# Script for building Breakpad

UPGRADE=0

while [[ $# -ge 1 ]]
do
key="$1"

case $key in
    -u|--upgrade)
        UPGRADE=1
    ;;
    -h|--help)
        echo "Possible options are:"
        echo " -u|--upgrade   Upgrades breakpad sources"
        echo " -h|--help      Prints this help message"
        exit 1
    ;;
    *)
        echo "Unknown options."
        exit 1
    ;;
esac
shift # past argument or value
done

set -e -x -v

export PROJECT_ROOT=`pwd`
export MAKEFLAGS=-j$(getconf _NPROCESSORS_ONLN)

if [ ! -d "depot_tools" ]; then
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
elif [ $UPGRADE -eq 1 ]; then
  pushd depot_tools
  git pull
  popd
fi

export PATH=${PROJECT_ROOT}/depot_tools:$PATH
echo "PREFIX: ${PREFIX:=${PROJECT_ROOT}/google-breakpad}"
mkdir -p ${PREFIX}

if [ ! -d "breakpad" ]; then
    # Checkout and build Breakpad
    mkdir -p breakpad
    cd breakpad
    fetch --nohooks breakpad
    cd src
    mkdir -p ${PREFIX}
elif [ $UPGRADE -eq 1 ]; then
    cd breakpad
    gclient sync --nohooks
    cd src
else
    cd breakpad/src
fi

rsync -a --exclude="*.git" ./src ${PREFIX}/
./configure --prefix=${PREFIX}
make install

cp    ${PROJECT_ROOT}/breakpad/src/src/third_party/libdisasm/libdisasm.a ${PREFIX}/lib/
cp -r ${PROJECT_ROOT}/breakpad/src/src/third_party/curl                  ${PREFIX}/include/breakpad/third_party/
