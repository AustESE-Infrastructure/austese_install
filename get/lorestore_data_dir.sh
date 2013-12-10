#!/bin/sh
. ./functions/global.sh
if [ ! -d /usr/share/lorestore ]; then
  ensure mkdir "/usr/share/lorestore"
fi
echo "/usr/share/lorestore"
exit 0
