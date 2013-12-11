#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
AUSTESEDB=`echo 'show databases' | mysql --user=root --password=jabberw0cky -B | grep austese`
if [ ! -z "$AUSTESEDB" ]; then
  read -p "Erase existing austese database?(y/n) " ANSWER
  if [ "$ANSWER" = "y" ]; then
    mysqladmin -f -u root -p$PASSWORD drop austese
  else
    echo "Exiting..."
    exit 1
  fi
fi
mysqladmin -u root -p$PASSWORD create austese
ensure mysql -u root -p$PASSWORD austese < ./objects/austese.sql
exit 0
