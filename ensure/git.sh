#!/bin/sh
. ./functions/global.sh
GIT=`which git`
if [ -z "$GIT" ]; then
  ensure apt-get -q -y install git
fi
exit 0
