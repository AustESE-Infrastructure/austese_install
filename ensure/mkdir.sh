#!/bin/sh
. ./functions/global.sh
if [ ! -d "$1" ]; then
  ensure mkdir "$1" 
fi
exit 0
