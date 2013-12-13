#!/bin/sh
. ./functions/global.sh
CATALINA_BASE=`./get/catalina_base.sh`
CATALINA_HOME=`./get/catalina_home.sh`
WADR="$CATALINA_BASE/webapps"
WUSR=`./get/apacheuser.sh`
WGRP=`./get/apachegroup.sh`
if [ -z "$WADR" ] || [ ! -d "$WADR" ]; then
  echo "webapps dir not found"
  exit 1
fi
TBIN="$CATALINA_HOME/bin"
if [ -z "$TBIN" ] || [ ! -d "$TBIN" ]; then
  echo "tomcat bin dir not found"
  exit 1
fi
ensure cp ./objects/calliope.war $WADR
ensure cp ./objects/LibPath.class $TBIN
ensure cp ./objects/setenv.sh $TBIN
exit 0
