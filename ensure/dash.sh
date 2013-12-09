#!/bin/sh
CURR_SHELL=`readlink /bin/sh`
if [ $CURR_SHELL = "dash" ]; then
  exit 0
else
  echo "This script does not work with $CURR_SHELL"
  exit 1
fi
