#!/bin/sh
. ./functions/global.sh
php_mod_enable() {
  appendto "/etc/php5/apache2/php.ini" "extension=$1.so"
  appendto "/etc/php5/cli/php.ini" "extension=$1.so"
}
PHP5ENMOD=`which php5enmod`
ENABLE_MOD=
if [ -z "$PHP5ENMOD" ]; then
  ENABLE_MOD=$PHP5ENMOD
else
  ENABLE_MOD=php_mod_enable
fi
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
  $ENABLE_MOD xsl
fi
PHP5CURL=`php5 -m | grep curl`
if [ -z "$PHP5CURL" ]; then
  ensure apt-get -qq install php5-curl
  $ENABLE_MOD curl
fi
IMAGICK=`php5 -m | grep imagick`
if [ -z "$IMAGICK" ]; then
  ensure apt-get -qq install php5-imagick
  $ENABLE_MOD imagick
fi
# ensure mongo gets installed on 12.04
MONGO=`php5 -m | grep mongo`
if [ -z "$MONGO" ]; then
  apt-get -qq install php5-mongo
  if [ $? -ne 0 ]; then
    pecl install mongo 2>/dev/null 1>/dev/null
    if [ $? -ne 0 ]; then
      $?=0
      $ENABLE_MOD mongo
    fi
  fi
  $ENABLE_MOD mongo
fi
MYSQL=`php5 -m | grep mysql`
if [ -z "$MYSQL" ]; then
  ensure apt-get -qq install php5-mysql
  $ENABLE_MOD mysql
fi
PHP5GD=`php5 -m | grep gd`
if [ -z "$PHP5GD" ]; then
  ensure apt-get -qq install php5-gd
  $ENABLE_MOD gd
fi
if [ ! -d "/usr/include/php5" ]; then
  ensure apt-get -qq install php5-dev
fi
exit 0
