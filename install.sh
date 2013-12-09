#!/bin/sh
. ./functions/global.sh
ensure ./install/environment.sh
read -p "Password for everything: (jabberw0cky)" PASSWORD
PASSWORD=${PASSWORD:-jabberw0cky}
WEBROOT=`./get/webroot.sh`
if [ $? -ne 0 ]; then
  echo "webroot ($WEBROOT) empty or not a directory"
  exit 1
fi
ensure ./install/apache.sh
ensure ./install/php5.sh
ensure ./install/mysql.sh $PASSWORD
ensure ./install/jdk.sh
ensure ./install/tomcat7.sh
ensure ./install/drupal.sh
#ensure ./install/mongo.sh $PASSWORD
#ensure ./install/mongo-1.4.1.sh
#ensure ./install/austese.sh
ensure cat > credentials.txt <<%%
Drupal username is austese
Mysql username is root
Mongo username is admin
Password for all is $PASSWORD
%%
echo "*** Login credentials written to credentials.txt"
echo "*** AustESE successfully installed at http://localhost/austese/"

