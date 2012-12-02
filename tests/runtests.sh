#!/bin/sh
OK="\033[40m\033[32m OK: "
ERR="\033[40m\033[31m FAIL: "
END="\033[0m"

CC=$(which gcc)
INCLUDES="-I../include/"
OPTS="-Wall -Wextra -pedantic --std=c99 -D_POSIX_C_SOURCE=200809L $INCLUDES"

cd $(dirname $0)

# readfile
$CC $OPTS -o readfile readfile.c ../shared/readfile.c ../shared/salloc.c
./readfile > tmpfile
COUNT=$(diff tmpfile /etc/hosts|wc -l)

if [ ! "$COUNT" = "0" ]; then
    echo "$ERR readfile $END"
else
    echo "$OK readfile $END"
fi

rm tmpfile readfile


# diff
$CC $OPTS -o tdiff diff.c ../shared/readfile.c ../shared/diff.c ../shared/salloc.c
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

#patch
$CC $OPTS -o tpatch patch.c ../shared/readfile.c ../shared/patch.c
diff testfile/diff1 testfile/diff2 > patchfile
./tpatch > out1

COUNT=$(diff -u out1 testfile/diff2|wc -l)

if [ ! "$COUNT" = "0" ]; then
    echo "$ERR patch $END"
else
    echo "$OK patch $END"
fi

rm out1 tpatch patchfile
