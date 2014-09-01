#!/bin/sh
. ./functions/global.sh
MYSQLD=`which mysqld`
if [ -z "$MYSQLD" ]; then
  ensure apt-get -qq install mysql-server
fi
MYSQLCLIENT=`which mysql`
if [ -z "$MYSQLCLIENT" ]; then
  ensure apt-get -qq install mysql-client
fi
ls /etc/apache2/mods-enabled/auth_mysql.load 1> /dev/null 2> /dev/null
if [ $? -ne 0 ]; then
  MOD_AUTH_MYSQL=`apt-cache search libapache2-mod.auth-mysql`
  if [ ! -z "$MOD_AUTH_MYSQL" ]; then
      ensure apt-get -qq install libapache2-mod-auth-mysql
  fi
fi
exit 0
