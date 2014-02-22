#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
AUSTESEDB=`echo 'show databases' | mysql --user=root --password=jabberw0cky -B | grep austese`
if [ ! -z "$AUSTESEDB" ]; then
  read -p "Erase existing austese database?(y/n) " ANSWER
  if [ "$ANSWER" = "y" ]; then
    ensure mysqladmin -f -u root -p$PASSWORD drop austese
  else
    echo "Exiting..."
    exit 1
  fi
fi
ensure mysqladmin -u root -p$PASSWORD create austese
ensure mysql -u root -p$PASSWORD austese < ./objects/austese.sql
mysql -u root -p$PASSWORD -e "delete from mysql.user where user='austese';"
if [ $? -ne 0 ]; then
  echo "failed to delete old austese user in mysql"
  exit 1
fi
mysql -u root -p$PASSWORD -e "flush privileges;"
if [ $? -ne 0 ]; then
  echo "failed to flush privileges in mysql"
  exit 1
fi
mysql -u root -p$PASSWORD -e "CREATE USER 'austese'@'localhost' IDENTIFIED BY '$PASSWORD';"
if [ $? -ne 0 ]; then
  echo "failed to create user austese in mysql"
  exit 1
fi
mysql -u root -p$PASSWORD -e "grant all privileges on *.* to 'austese'@'localhost';"
if [ $? -ne 0 ]; then
  echo "failed to grant privileges to use austese in mysql"
  exit 1
fi

exit 0
