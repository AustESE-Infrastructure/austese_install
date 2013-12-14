#!/bin/sh
. ./functions/global.sh
ensure ./install/environment.sh
read -p "Password for everything: (jabberw0cky)" PASSWORD
PASSWORD=${PASSWORD:-jabberw0cky}
ensure ./install/apache.sh
ensure ./install/php5.sh
ensure ./install/mysql.sh $PASSWORD
ensure ./install/jdk.sh
ensure ./install/tomcat7.sh
ensure ./install/drupal.sh $PASSWORD
ensure ./install/mongo.sh $PASSWORD
ensure ./install/austese.sh $PASSWORD
ensure ./install/lorestore.sh $PASSWORD
ensure ./install/calliope.sh $PASSWORD
ensure ./install/calliope-libs.sh
ensure ./install/proxy.sh
ensure apachectl restart
ensure /etc/init.d/tomcat7 restart
ensure cat > credentials.txt <<%%
Drupal username is austese
Mysql username is austese 
Mongo username is admin
Lorestore username is lorestore
Password for all is $PASSWORD
%%
echo "*** Login credentials written to credentials.txt"
echo "*** AustESE successfully installed at http://localhost/austese/"

