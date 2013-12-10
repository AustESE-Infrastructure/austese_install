#!/bin/sh
. ./functions/global.sh
if [ ! -d "/var/lib/tomcat7/conf" ]; then
  echo "installing tomcat7"
  ensure apt-get -qq install tomcat7
fi
if [ ! -d "/usr/share/tomcat7/bin" ]; then
  echo "installing tomcat7-common"
  ensure apt-get -qq install tomcat7-common
fi
if [ ! -f "/usr/share/doc-base/tomcat7" ]; then
  echo "installing tomcat7-docs"
  ensure apt-get -qq install tomcat7-docs
fi
if [ ! -d "/usr/share/doc/tomcat7-admin" ]; then
  echo "instaling tomcat7-admin"
  ensure apt-get -qq install tomcat7-admin
fi
if [ ! -d "/usr/share/doc/tomcat7-user" ]; then
  echo "installing tomcat7-user"
  ensure apt-get -qq install tomcat7-user
fi
exit 0
