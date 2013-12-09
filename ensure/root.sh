#!/bin/sh
if [ "$USER" != "root" ]; then
  echo "This script requires root access"
  exit 1
else
  exit 0
fi
