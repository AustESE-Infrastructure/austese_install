#!/bin/sh
. ./functions/global.sh
UNZIP=`which unzip`
if [ -z "$UNZIP" ]; then
  ensure apt-get -q -y install unzip
fi
exit 0
