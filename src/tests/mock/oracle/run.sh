#!/bin/sh
[ -z "$1" ] && APPDIR="/app/src/" || APPDIR="$1"
. /app/src/tests/system/common.sh
DIR="oracle"

# TLS configuration
$APPDIR/bin/alligator $APPDIR/tests/mock/$DIR/alligator.conf&
sleep 15

TEXT=`curl -s localhost:1111`
METRICS=`cat $APPDIR/tests/mock/$DIR/metrics.txt`
for NAME in $METRICS
do
	echo "$TEXT" | grep $NAME >/dev/null 2>&1 && success $NAME || error "$TEXT" $NAME
done

pkill alligator
