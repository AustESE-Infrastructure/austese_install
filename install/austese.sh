#!/bin/sh
. ./functions/global.sh
mod_install () {
# 1 is the module name
# 2 is the webroot
ensure wget -P /tmp https://github.com/uq-eresearch/$1
ensure mkdir "$2/sites/all/modules/$1"
ensure unzip -d /tmp/master.zip $2/austese/sites/all/modules/$1/
ensure drush en $1 -y
exit 0
}
SAVEDPWD=`pwd`
WEBROOT=`./get/webroot.sh`
if [ -z "$WEBROOT" ]; then
  echo "failed to find webroot"
  exit 1
fi
WUSER=`./get/apacheuser.sh`
if [ -z "$WUSER" ]; then
  echo "failed to find webuser"
  exit 1
fi
WGROUP=`./get/apachegroup.sh`
if [ -z "$WGROUP" ]; then
  echo "failed to find webgroup"
  exit 1
fi
ensure cd $WEBROOT/austese/
pwd
ensure drush dl og -y
ensure drush en og og_access og_ui og_context -y
ensure drush dl og_invite_people -y
ensure drush en og_invite_people -y
ensure drush dl jquery_update -y
ensure drush en jquery_update -y
ensure drush dl views -y
ensure drush en views -y
ensure drush en repositoryapi -y
ensure mod_install austese_repository $WEBROOT  
ensure mod_install austese_alignment $WEBROOT
ensure mod_install austese_annotations $WEBROOT
ensure mod_install austese_collation $WEBROOT
ensure mod_install austese_alignment $WEBROOT
ensure mod_install austese_lightbox $WEBROOT
# enable access to all for web user
ensure chown -R $WUSER $WEBROOT/austese/sites/all/modules/
ensure chgrp -R $WGROUP $WEBROOT/austese/sites/all/modules/
ensure cd $SAVEDPWD
exit 0
