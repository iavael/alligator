#!/bin/sh
[ -z "$1" ] && APPDIR="/app/src/" || APPDIR="$1"
. /app/src/tests/system/common.sh
DIR="named"

\cp -f $APPDIR/tests/system/$DIR/named.conf /etc/
\cp -f $APPDIR/tests/system/$DIR/named.rfc1912.zones /etc/
named -c /etc/named.conf
host localhost localhost >/dev/null
$APPDIR/bin/alligator $APPDIR/tests/system/$DIR/alligator.yaml&
sleep 5

TEXT=`curl -s localhost:1111`
METRICS=`cat $APPDIR/tests/system/$DIR/metrics.txt`
for NAME in $METRICS
do
	echo "$TEXT" | grep $NAME >/dev/null 2>&1 && success $NAME || error "$TEXT" $NAME
done

kill %1

$APPDIR/bin/alligator $APPDIR/tests/system/$DIR/alligator.json&
sleep 15

TEXT=`curl -s localhost:1111`
METRICS=`cat $APPDIR/tests/system/$DIR/metrics.txt`
for NAME in $METRICS
do
	echo "$TEXT" | grep $NAME >/dev/null 2>&1 && success $NAME || error "$TEXT" $NAME
done

kill %2

$APPDIR/bin/alligator $APPDIR/tests/system/$DIR/alligator.conf&
sleep 15

TEXT=`curl -s localhost:1111`
METRICS=`cat $APPDIR/tests/system/$DIR/metrics.txt`
for NAME in $METRICS
do
	echo "$TEXT" | grep $NAME >/dev/null 2>&1 && success $NAME || error "$TEXT" $NAME
done

kill %3
pkill named
