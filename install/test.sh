#!/bin/sh
CMD=""
for var in "$@"
do
    CMD="$CMD $var"
done
echo $CMD
exit 0
