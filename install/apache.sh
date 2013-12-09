#!/bin/sh
. ./functions/global.sh
APACHE=`which apache2`
if [ -z "$APACHE" ]; then
  ensure apt-get -qq install apache2
fi
ENABLED=0
PROXY_HTTP=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep proxy_http_module`
if [ -z "$PROXY_HTTP" ]; then
  ensure a2enmod proxy_http
  ENABLED=1
fi
PROXY=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep proxy_module`
if [ -z "$PROXY" ]; then
  ensure a2enmod proxy
  ENABLED=1
fi
REWRITE=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep rewrite_module`
if [ -z "$REWRITE" ]; then
  ensure a2enmod rewrite
  ENABLED=1
fi
HEADERS=`apachectl -t -D DUMP_MODULES 2>/dev/null | grep headers_module`
if [ -z "$HEADERS" ]; then
  ensure a2enmod headers
  ENABLED=1
fi
if [ $ENABLED -eq 1 ]; then
  ensure apachectl restart
fi
exit 0
