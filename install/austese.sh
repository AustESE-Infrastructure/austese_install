#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
ensure mysql -u root -p$PASSWORD austese < ./objects/austese.sql
exit 0
