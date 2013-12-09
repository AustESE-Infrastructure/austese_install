#!/bin/sh
. ./functions/global.sh
WGET=`which wget`
if [ -z "$WGET" ]; then
  ensure apt-get -q -y install wget
fi
exit 0
