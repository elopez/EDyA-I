#!/bin/sh
CC=$(which gcc)
INCLUDES="-I../include/"
OPTS="-Wall -Wextra -pedantic $INCLUDES"

cd $(dirname $0)

# readfile
$CC $OPTS -o readfile readfile.c ../libs/readfile.c
./readfile > tmpfile
COUNT=$(diff tmpfile /etc/hosts|wc -l)

if [ ! "$COUNT" = "0" ]; then
    echo "FAIL: readfile"
else
    echo "OK: readfile"
fi

rm tmpfile readfile
