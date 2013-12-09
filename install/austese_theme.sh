#!/bin/sh
. ./funtions/global.sh
WEBROOT=`./get/webroot.sh`
if [ -z "$WEBROOT" ]; then
  echo "web root not found"
  exit 1
fi
ensure wget -P /tmp https://github.com/uq-eresearch/austese_theme/archive/master.zip
ensure unzip -d $WEBROOT/austese/sites/all/themes/ /tmp/master.zip
ensure wget -P /tmp http://getbootstrap.com/2.3.2/assets/bootstrap.zip
ensure unzip -d $WEBROOT/austese/sites/all/themes/austese/
exit 0
