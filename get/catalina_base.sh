#!/bin/sh
PS_RAW=`ps aux | grep tomcat`
CATALINA_BASE=`echo $PS_RAW | awk '$1~/tomcat/{for(i=2;i<NF;i++){if($i~/-Dcatalina.base/){split($i,a,"=");print a[2]}}}'`
echo $CATALINA_BASE
