#!/bin/sh
. ./functions/global.sh
PASSWORD=$1
mysqladmin -f -u root -p$PASSWORD drop austese
mysqladmin -u root -p$PASSWORD create austese
ensure mysql -u root -p$PASSWORD austese < ./objects/austese.sql
exit 0
