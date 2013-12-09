#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
ensure apt-key adv --keyserver keyserver.ubuntu.com --recv 7F0CEB10
ensure echo 'deb http://downloads-distro.mongodb.org/repo/ubuntu-upstart dist 10gen' | sudo tee /etc/apt/sources.list.d/mongodb.list
ensure apt-get -qq update
ensure apt-get -qq install mongodb-10gen
mongo << %%
db = db.getSiblingDB('admin')
if ( db.system.users.find({user: "admin"}).count()==0 )
{
  db.addUser('admin', "$PASSWORD", 'userAdminAnyDatabase')
}
%%
if [ $? -ne 0 ] then
  echo "failed to set mongo username and password"
  exit 1
fi
ensure pecl install mongo-1.4.1
ensure pecl install uploadprogress
appendto /etc/php5/cli/php.ini extension=uploadprogress.so
appendto /etc/php5/apache2/php.ini extension=uploadprogress.so
#now preload the mongo database
ensure mongorestore --host localhost --username admin --password $PASSWORD ./install/dump
exit 0
