#!/bin/sh
. ./functions/global.sh
TIDYPROG=`which tidy`
if [ -z "$TIDYPROG" ]; then
  ensure apt-get -qq install tidy
fi
if [ ! -f "/usr/include/tidy/tidy.h" ]; then
  ensure apt-get -qq install libtidy-dev
fi
exit 0
