#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
AUSTESEDB=`echo 'show databases' | mysql --user=root --password=jabberw0cky -B | grep austese`
if [ ! -z "$AUSTESEDB" ]; then
  read -p "Erase existing austese database?(y/n) " ANSWER
  if [ "$ANSWER" = "y" ]; then
    ensure mysqladmin -f -u root -p$PASSWORD drop austese
    ensure mysqladmin -u root -p$PASSWORD create austese
    ensure mysql -u root -p$PASSWORD austese < ./objects/austese.sql
  else
    echo "Exiting..."
    exit 1
  fi
fi
mysql -u root -p$PASSWORD -e "delete from mysql.user where user='austese';"
mysql -u root -p$PASSWORD -e "flush privileges;"
mysql -u root -p$PASSWORD -e "CREATE USER 'austese'@'localhost' IDENTIFIED BY '$PASSWORD';"
mysql -u root -p$PASSWORD -e "grant all privileges on *.* to 'austese'@'localhost';"
exit 0
