#!/bin/sh
. ./functions/global.sh
WADR=`./get/tomcat_webapps_dir.sh`
WUSR=`./get/apacheuser.sh`
WGRP=`./get/apachegroup.sh`
LODD=`./get/lorestore_data_dir.sh`
if [ -z "$WADR" ] || [ -z "$WUSR" ] || [ -z "$WGRP" ] || [ -z "$LODD" ]; then
  echo "webapps=$WADR apache user=$WUSR apache group=$WGRP lorestore dd=$LODD"
  exit 1
fi
ensure wget -q -r -nc -nd -P /tmp https://dl.dropbox.com/u/8415460/tmp/lorestore.war
if [ -d "$WADR" ]; then
  ensure cp /tmp/lorestore.war $WADR
else
  echo "tomcat webapps directory not in expected location"
  exit 1
fi
ensure chown -R $WUSR $LODD
ensure chgrp -R $WGRP $LODD
exit 0
