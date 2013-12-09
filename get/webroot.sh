#!/bin/sh
if [ ! -f /etc/apache2/sites-available/default ]; then
  echo "/etc/apache2/sites/available/default not found"
  exit 1
fi
WEBROOT=`grep -i 'DocumentRoot' /etc/apache2/sites-available/default | awk '{print $2}'`
if  [ -z "$WEBROOT" ]; then
  echo "web root not found"
  exit 1
fi
echo $WEBROOT
exit 0
