#!/bin/sh
. ./functions/global.sh
WEBROOT=`./get/webroot.sh`
if [ -z $WEBROOT ]; then
  echo "WEBROOT=$WEBROOT failed"
  exit 1
fi
ensure wget -P /tmp "http://www.sencha.com/products/extjs/download/ext-js-4.1.1/1683"
if [ ! -d "$WEBROOT/austese/sites/all/libraries" ]; then
  ensure mkdir $WEBROOT/austese/sites/all/libraries/
fi
ensure unzip -q -d $WEBROOT/austese/sites/all/libraries/ ext-4.1.1a-gpl.zip
exit 0
