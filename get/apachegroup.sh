#!/bin/sh
if [ -f /etc/apache2/envvars ]; then
  `grep -i 'APACHE_RUN_GROUP' /etc/apache2/envvars`
  if [ -z "$APACHE_RUN_GROUP" ]; then
    echo "Apache group not found"
    exit 1
  fi
else
  echo "/etc/apache2/envvars not found"
  exit 1
fi
echo $APACHE_RUN_GROUP
exit 0

