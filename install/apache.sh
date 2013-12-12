#!/bin/sh
. ./functions/global.sh
APACHE=`which apache2`
APACHE_ETC=`./get/apacheetc.sh`
if [ -z "$APACHE_ETC" ]; then
  echo "couldn't find apache etc dir"
  exit 1
fi
if [ -z "$APACHE" ]; then
  ensure apt-get -qq install apache2
fi
PROXY_HTTP=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep proxy_http_module`
if [ -z "$PROXY_HTTP" ]; then
  ensure a2enmod proxy_http
fi
PROXY=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep proxy_module`
if [ -z "$PROXY" ]; then
  ensure a2enmod proxy
fi
REWRITE=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep rewrite_module`
if [ -z "$REWRITE" ]; then
  ensure a2enmod rewrite
fi
HEADERS=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep headers_module`
if [ -z "$HEADERS" ]; then
  ensure a2enmod headers
fi
if [ -f "$APACHE_ETC/sites-available/default" ]; then
  ensure mv $APACHE_ETC/sites-available/default $APACHE_ETC/sites-available/default-old
fi
ensure cp ./objects/default $APACHE_ETC/sites-available/
exit 0
