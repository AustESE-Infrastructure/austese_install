#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
if [ -z "$PASSWORD" ]; then
  echo "password is empty!"
  exit 1
fi
ensure apt-key adv --keyserver keyserver.ubuntu.com --recv 7F0CEB10 2>&1  1>/dev/null
echo 'deb http://downloads-distro.mongodb.org/repo/ubuntu-upstart dist 10gen' > /etc/apt/sources.list.d/mongodb.list
ensure apt-get -qq update 2>&1 1>/dev/null
ensure apt-get -qq install mongodb-10gen 2>&1 1>/dev/null
mongo << %%
use admin
if ( db.system.users.find({user: "admin"}).count()==0 ) {
  db.addUser('admin', "$PASSWORD", 'userAdminAnyDatabase')
}
%%
if [ $? -ne 0 ]; then
  echo "failed to set mongo username and password"
  exit 1
fi
MONGO_VERSION=`pecl info mongo | grep "Release Version" | awk '{print $3}'`
if [ "$MONGO_VERSION" != "1.4.1" ]; then
  ensure pecl install mongo-1.4.1
  ensure php5enmod mongo
fi
UP_VERSION=`pecl info uploadprogress | grep "Release Version" | awk '{print $3}'`
if [ -z "$UP_VERSION" ]; then
  ensure pecl install uploadprogress
  ensure php5enmod uploadprogress
fi
#now preload the mongo database
ensure mongorestore --host localhost --username admin --password $PASSWORD ./objects/dump 2>&1 1>/dev/null
exit 0
