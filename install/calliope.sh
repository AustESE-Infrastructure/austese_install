#!/bin/sh
. ./functions/global.sh
WADR=`./get/tomcat_webapps_dir.sh`
if [ -z "$WADR" ] || [ ! -d "$WADR" ]; then
  echo "webapps dir not found"
  exit 1
fi
TBIN=`./get/tomcat_bin_dir.sh`
if [ -z "$TBIN" ] || [ ! -d "$TBIN" ]; then
  echo "tomcat bin dir not found"
  exit 1
fi
ensure wget -q -nc -r -nd -P /tmp https://github.com/AustESE-Infrastructure/calliope/blob/master/calliope.war
ensure wget -q -nc -r -nd -P /tmp https://github.com/AustESE-Infrastructure/calliope/blob/master/tomcat-bin/setenv.sh
ensure wget -q -nc -r -nd -P /tmp https://github.com/AustESE-Infrastructure/calliope/blob/master/tomcat-bin/LibPath.class
ensure cp /tmp/calliope.war $WADR
ensure cp /tmp/LibPath.class $TBIN
ensure cp /tmp/setenv.sh $TBIN
exit 0
