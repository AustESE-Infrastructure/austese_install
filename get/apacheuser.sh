#!/bin/sh
if [ -f /etc/apache2/envvars ]; then
  `grep -i 'APACHE_RUN_USER' /etc/apache2/envvars`
  if [ -z "$APACHE_RUN_USER" ]; then
    echo "Apache user not found"
    exit 1
  fi
else
  echo "/etc/apache2/envvars not found"
  exit 1
fi
echo $APACHE_RUN_USER
exit 0
