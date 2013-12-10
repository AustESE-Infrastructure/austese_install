#!/bin/sh
if [ ! -d /etc/apache2/mods-enabled ]; then
  echo "apache mods-enabled dir not found"
  exit 1
fi
echo "/etc/apache2/mods-enabled"
exit 0
