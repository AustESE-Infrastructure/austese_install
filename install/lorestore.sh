#!/usr/sh
. ./functions/global.sh
WEBAPPS=`./get/tomcat_webapps_dir.sh`
ensure wget -P /tmp https://dl.dropbox.com/u/8415460/tmp/lorestore.war
if [ -d "$WEBAPPS" ]; then
  ensure cp /tmp/lorestore.war $WEBAPPS
else
  echo "tomcat webapps directory not in expected location"
  exit 1
fi
