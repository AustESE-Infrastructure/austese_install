#!/bin/sh
. ./functions/global.sh
#php5 itself
PHP5=`which php5`
if [ -z "$PHP5" ]; then
  ensure apt-get -qq install php5
fi
PHP5XSL=`php5 -m | grep xsl`
if [ -z "$PHP5XSL" ]; then
  ensure apt-get -qq install php5-xsl
fi
PHP5CURL=`php5 -m | grep curl`
if [ -z "$PHP5CURL" ]; then
  ensure apt-get -qq install php5-curl
fi
IMAGICK=`php5 -m | grep imagick`
if [ -z "$IMAGICK" ]; then
  ensure apt-get -qq install php5-imagick
fi
MONGO=`php5 -m | grep mongo`
if [ -z "$MONGO" ]; then
  ensure apt-get -q -y install php5-mongo
  php5enmod mongo
fi
MYSQL=`php5 -m | grep mysql`
if [ -z "$MYSQL" ]; then
  ensure apt-get -qq install php5-mysql
fi
PHP5GD=`php5 -m | grep gd`
if [ -z "$PHP5GD" ]; then
  ensure apt-get -qq install php5-gd
fi
if [ ! -d "/usr/include/php5" ]; then
  ensure apt-get -qq install php5-dev
fi
if [ ! -d "/usr/share/php/PEAR" ]; then
  ensure apt-get -qq install php-pear
fi
exit 0
