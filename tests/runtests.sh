#!/bin/sh
OK="\033[40m\033[32m OK: "
ERR="\033[40m\033[31m FAIL: "
END="\033[0m"

CC=$(which gcc)
INCLUDES="-I../include/"
OPTS="-Wall -Wextra -pedantic --std=c99 $INCLUDES"

cd $(dirname $0)

# readfile
$CC $OPTS -o readfile readfile.c ../libs/readfile.c
./readfile > tmpfile
COUNT=$(diff tmpfile /etc/hosts|wc -l)

if [ ! "$COUNT" = "0" ]; then
    echo "$ERR readfile $END"
else
    echo "$OK readfile $END"
fi

rm tmpfile readfile


# diff
$CC $OPTS -o tdiff diff.c ../libs/readfile.c ../libs/diff.c
./tdiff > out1
diff testfile/diff* > out2

cp testfile/diff1 patchme1
cp testfile/diff1 patchme2

patch patchme1 out1 >/dev/null
patch patchme2 out2 >/dev/null

COUNT=$(diff -u patchme1 patchme2|wc -l)

if [ ! "$COUNT" = "0" ]; then
    echo "$ERR diff $END"
else
    echo "$OK diff $END"
fi

rm out1 out2 tdiff patchme*
