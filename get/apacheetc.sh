#!/bin/sh
if [ ! -d "/etc/apache2/" ]; then
  echo "/etc/apache2/ not found"
  exit 1
fi
echo  "/etc/apache2"
exit 0
