#!/bin/sh
. ./functions/global.sh
READLINK=`which readlink`
if [ -z "$READLINK" ]; then
  ensure apt-get -q -y install coreutils
fi
exit 0
