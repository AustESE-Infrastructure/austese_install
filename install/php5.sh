#!/bin/sh
. ./functions/global.sh
PHP5ENMOD=`which php5enmod`
#php5 itself
PHP5=`which php5`
if [ -z "$PHP5" ]; then
  ensure apt-get -qq install php5
fi
if [ ! -d "/usr/share/php/PEAR" ]; then
  ensure apt-get -qq install php-pear
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
# ensure mongo gets installed on 12.04
MONGO=`php5 -m | grep mongo`
if [ -z "$MONGO" ]; then
  apt-get --simulate -qq install php5-mongo
  if [ $? -ne 0 ]; then
    pecl install mongo 2>/dev/null 1>/dev/null
  else
    apt-get -qq install php5-mongo
  fi
  appendto "/etc/php5/apache2/php.ini" "extension=mongo.so"
  appendto "/etc/php5/cli/php.ini" "extension=mongo.so"
fi
MYSQL=`php5 -m | grep mysql`
UP_VERSION=`pecl info uploadprogress | grep "Release Version" | awk '{print $3}'`
if [ -z "$UP_VERSION" ]; then
  ensure pecl install uploadprogress
  appendto "/etc/php5/apache2/php.ini" "extension=uploadprogress.so"
  appendto "/etc/php5/cli/php.ini" "extension=uploadprogress.so"
fi
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
exit 0
