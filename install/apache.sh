#!/bin/sh
. ./functions/global.sh
APACHE=`which apache2`
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
exit 0
