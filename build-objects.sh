#!/bin/sh
. ./functions/global.sh
WEBROOT=`./get/webroot.sh`
read -p "Mysql root password:" PASSWORD
PASSWORD=${PASSWORD:-random}
OLDPWD=`pwd`
cd objects
ensure rm -f austese.tar.gz.*
ensure tar czf austese.tar.gz -C $WEBROOT austese/ 
ensure split -a 1 -n 5 -d austese.tar.gz austese.tar.gz.
ensure rm austese.tar.gz
cd $OLDPWD
# dump austese database
ensure mysqldump -u root -p$PASSWORD austese > objects/austese.sql
