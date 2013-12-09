#!/bin/sh
. ./functions/global.sh
JAVAC=`which javac`
if [ -z "$JAVAC" ]; then
  ensure apt-get -qq install default-jdk
else
  JVERS=`$JAVAC -version 2>&1`
  oldIFS=$IFS
  IFS='.'
  set -- $JVERS
  if [ $2 != "6" ] && [ $2 != "7" ]; then
    ensure apt-get -qq install default-jdk
  fi
fi
IFS=$oldIFS
exit 0

