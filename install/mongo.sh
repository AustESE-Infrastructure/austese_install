#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
if [ -z "$PASSWORD" ]; then
  echo "password is empty!"
  exit 1
fi
MONGODBV=`mongo --version | awk '{print $4}'`
if [ -z "$MONGODBV" ] || [ "$MONGODBV" \< "2.4.8" ]; then
  apt-key adv --keyserver keyserver.ubuntu.com --recv 7F0CEB10 
  ensure rm -f /etc/apt/sources.list.d/mongodb.list
  echo 'deb http://downloads-distro.mongodb.org/repo/ubuntu-upstart dist 10gen' > /etc/apt/sources.list.d/mongodb.list
  ensure apt-get -qq update 2>&1 1>/dev/null
  if [ ! -z "$MONGODBV" ]; then
    apt-get remove mongodb mongodb-clients
  fi
  ensure apt-get -qq install mongodb-10gen 2>&1 1>/dev/null
fi
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
#now preload the mongo database
ensure mongorestore --host localhost --username admin --password $PASSWORD ./objects/dump 2>&1 1>/dev/null
exit 0
