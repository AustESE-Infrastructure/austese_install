#!/bin/sh
. ./functions/global.sh
WEBROOT=`./get/webroot.sh`
ANSWER="n"
if [ -d "$WEBROOT/austese/" ]; then
  read -p "Erasing existing $WEBROOT/austese directory?(y/n) " ANSWER
fi
if [ "$ANSWER" = "y" ]; then
  rm -rf "$WEBROOT/austese"
elif [ -d "$WEBROOT/austese" ]; then
  echo "Exiting..."
  exit 1
fi
ensure cat ./objects/austese.tar.gz.* | tar xz -C "$WEBROOT"
ensure chown -R www-data "$WEBROOT/austese"
ensure chgrp -R www-data "$WEBROOT/austese"
exit 0
