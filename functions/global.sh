ensure() {
CMD=""
for var in "$@"
do
  if [ -z "$CMD" ]; then
    CMD=$var
  else
    CMD="$CMD $var"
  fi
done
$CMD
if [ $? -ne 0 ]; then
  echo "$CMD failed"
  exit 1
fi
}
appendto() {
FOUND=""
if [ -f $1 ]; then
  FOUND=`grep "$2" $1`
else
  echo "warning: could not find $1"
fi
if [ -z "$FOUND" ]; then
  echo "$2" >> $1
fi
}
