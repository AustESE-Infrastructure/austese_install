#!/bin/sh
. ./functions/global.sh
ensure chgrp $1 -R $2
exit 0

