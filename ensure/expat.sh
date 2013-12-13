#!/bin/sh
. ./functions/global.sh
if [ ! -f "/usr/include/expat.h" ]; then
  ensure apt-get -qq install libexpat1
  ensure apt-get -qq install libexpat1-dev
fi
exit 0
