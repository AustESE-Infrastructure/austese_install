#!/bin/sh
if [ ! -f "/etc/debian_version" ]; then
  echo "This installer only works on Debian/Ubuntu"
  exit 1
else
  exit 0
fi
