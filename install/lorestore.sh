#!/bin/sh
. ./functions/global.sh
CATALINA_BASE=`./get/catalina_base.sh`
WADR="$CATALINA_BASE/webapps"
WUSR=`./get/apacheuser.sh`
WGRP=`./get/apachegroup.sh`
LODD=`./get/lorestore_data_dir.sh`
if [ -z "$WADR" ] || [ -z "$WUSR" ] || [ -z "$WGRP" ] || [ -z "$LODD" ]; then
  echo "webapps=$WADR apache user=$WUSR apache group=$WGRP lorestore dd=$LODD"
  exit 1
fi
if [ -d "$WADR" ]; then
  ensure cp ./objects/lorestore.war $WADR
  ensure chown tomcat7 $WADR/lorestore.war
  ensure chgrp tomcat7 $WADR/lorestore.war
else
  echo "tomcat webapps directory not in expected location"
  exit 1
fi
ensure chown -R $WUSR $LODD
ensure chgrp -R $WGRP $LODD
exit 0
