#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
if [ -z "$PASSWORD" ]; then
  echo "PASSWORD to calliope.sh cannot be empty"
  exit 1
fi
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
# embed users password into calliope web.xml
ensure rm -rf /tmp/calliope
ensure mkdir /tmp/calliope
ensure cp ./objects/calliope.war /tmp/calliope
CPWD=`pwd`
ensure cd /tmp/calliope
ensure jar xf calliope.war
ensure cd ..
ensure sed -i -e "s/jabberw0cky/$PASSWORD/" /tmp/calliope/WEB-INF/web.xml
jar cf calliope.war -C calliope WEB-INF
cd "$CPWD"
ensure cp /tmp/calliope.war $WADR
ensure cp ./objects/LibPath.class $TBIN
ensure cp ./objects/setenv.sh $TBIN
exit 0
