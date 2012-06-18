#!/bin/sh
CC=$(which gcc)
INCLUDES="-I../include/"

cd $(dirname $0)

# readfile
$CC $INCLUDES -o readfile readfile.c ../libs/readfile.c
./readfile > tmpfile
COUNT=$(diff tmpfile /etc/hosts|wc -l)

if [ ! "$COUNT" = "0" ]; then
    echo "FAIL: readfile"
else
    echo "OK: readfile"
fi

rm tmpfile readfile
