#!/bin/sh
. ./functions/global.sh
CPWD=`pwd`
cd objects/formatter/
ensure `./rebuildll.sh`
cd ../speller
ensure `./rebuildll.sh`
cd ../stripper
ensure `./rebuildll.sh`
cd "$CPWD"
exit 0
