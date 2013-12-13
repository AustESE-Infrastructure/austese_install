#!/bin/sh
. ./functions/global.sh
GCC_BIN=`which gcc`
if [ ! -z "$GCC_BIN" ]; then
  GCC_VERS=`gcc --version | awk '$1~/gcc/ {print $NF}'`
  if [ "$GCC_VERS" \< "4.6.0" ]; then
    ensure apt-get -qq install gcc
  fi
else
  ensure apt-get -qq install gcc
fi
