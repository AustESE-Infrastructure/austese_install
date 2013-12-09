#!/bin/sh
. ./functions/global.sh
MAKE=`which make`
if [ -z "$MAKE" ]; then
  ensure apt-get -q -y install make
fi
exit 0
