#!/bin/sh

######## CONFIGURATION ########
GITDIR=$(echo ~/Proyecto)
MYREPODIR=$(echo ~/test)
###### END CONFIGURATION ######

if [ ! -d "$GITDIR/.git/" ]; then
    echo "GITDIR is not a git repository! Aborting"
    exit 1
fi

if [ ! -d "$MYREPODIR" ]; then
    echo "MYREPODIR is not a valid directory! Aborting"
    exit 1
fi


VG="valgrind --tool=memcheck --leak-check=full --error-exitcode=1 -q"

function abort_debug(){
    echo $FILE
    echo $i
    exit 1
}

cd $GITDIR
COMMITS=$(git log | grep '^commit' | tac | sed 's/commit/ /')

cd $MYREPODIR
myrepo init

for i in $COMMITS; do
    FILE=$(mktemp)

    cd $GITDIR
    git show "$i" > $FILE

    cd $MYREPODIR
    patch -p1 < $FILE
    $VG myrepo add . 2>&1 || abort_debug
    $VG myrepo status 2>&1 || abort_debug
    $VG myrepo commit -m "Git commit $i" 2>&1 || abort_debug

    rm $FILE
done;
