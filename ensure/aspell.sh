#!/bin/sh
. ./functions/global.sh
ASPELL_PROG=`which aspell`
if [ -z "ASPELL_PROG" ]; then
  ensure apt-get -qq install aspell
fi
if [ ! -f "/usr/include/aspell.h" ]; then
  ensure apt-get -qq install libaspell-dev
fi
exit 0
