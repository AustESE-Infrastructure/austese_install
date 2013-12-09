#!/bin/sh
. ./functions/global.sh
DRUSH=`which drush`
if [ -z "$DRUSH" ]; then
  ensure apt-get install drush
fi
exit 0
