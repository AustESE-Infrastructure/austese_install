#!/bin/sh
PS_RAW=`ps aux | grep tomcat`
CATALINA_HOME=`echo $PS_RAW | awk '$0~/tomcat/{for(i=2;i<NF;i++){if($i~/-Dcatalina.home/){split($i,a,"=");print a[2]}}}'`
echo $CATALINA_HOME
