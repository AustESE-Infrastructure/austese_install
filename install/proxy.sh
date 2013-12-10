#!/bin/sh
. ./functions/global.sh
ACD=`./get/apache_conf_dir.sh`
if [ $? -ne 0 ]; then
  echo "apache config dir not found"
  exit 1
fi
if [ -f "$ACD/proxy.conf" ]; then
  ensure mv "$ACD/proxy.conf" "$ACD/proxy.conf.old"
fi
ensure cp ./objects/proxy.conf $ACD
exit 0
