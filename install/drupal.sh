#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
if [ -z "$PASSWORD" ]; then
  echo "password is empty: install/drupal.sh"
  exit 1
fi
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
SETTINGS="$WEBROOT/austese/sites/default/settings.php"
ensure cat ./objects/austese.tar.gz.* | tar xz -C "$WEBROOT"
# change database password to that supplied by user
ensure chmod 777 $SETTINGS
sed "s/'password' => 'austese9875\!',/'password' => '$PASSWORD',/" <$SETTINGS >/tmp/settings.php
cp /tmp/settings.php $SETTINGS
if [ $? -ne 0 ]; then
  echo "sed (2) failed on settings.php"
  exit 1
fi
ensure chmod 444 $SETTINGS
ensure chown -R www-data "$WEBROOT/austese"
ensure chgrp -R www-data "$WEBROOT/austese"
exit 0
