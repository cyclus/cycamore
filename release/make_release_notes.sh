#!/bin/bash

set -e

die() {
    echo >&2 "$@"
    exit 1
}

# check input
CORE=${CORE_DIR?"Environment variable CORE_DIR must be set to the cyclus repository directory."}
CYCA=${CYCAMORE_DIR?"Environment variable CYCAMORE_DIR must be set to the cycamore repository directory."}
[ "$#" -eq 2 ] || die "Input requires from version and to version, e.g., 
$ ./make_release_notes.sh 0.2.0 0.3.0
" 
PREV=$1
VERSION=$2
echo "Making release notes template for Cyclus stack verison $VERSION from $PREV"

# setup
FILE=release_notes.rst
cp release_notes.rst.in $FILE
HERE=$PWD
sed -i "s/@PREV_VERSION@/$PREV/g" $FILE 
sed -i "s/@VERSION@/$VERSION/g" $FILE 

# cyclus summary
cd $CORE
NCOMMITS=`git rev-list 1.2.0...1.3.0-rc1 --count | tail -n1`
SUMMARY=`git diff --stat 1.2.0...1.3.0-rc1 | tail -n1`
CORECONTRIB=`git log --format='%aN' 1.2.0...1.3.0-rc1 | sort -u`
cd $HERE
TXT="$NCOMMITS commits resulting in $SUMMARY"
echo "txt: $TXT"
sed -i "s/@CORE_SUMMARY@/$TXT/g" $FILE 

# cycamore summary
cd $CYCA
NCOMMITS=`git rev-list 1.2.0...1.3.0-rc1 --count | tail -n1`
SUMMARY=`git diff --stat 1.2.0...1.3.0-rc1 | tail -n1`
CYCACONTRIB=`git log --format="%aN" 1.2.0...1.3.0-rc1 | sort -u`
cd $HERE
TXT="$NCOMMITS commits resulting in $SUMMARY"
sed -i "s/@CYCA_SUMMARY@/$TXT/g" $FILE 

# contributors, beware, thar be hackery here
echo "$CORECONTRIB" > .contribs
echo "$CYCACONTRIB" >> .contribs
TXT=`cat .contribs | sort -u | awk '{print "* " $0}'`
echo "$TXT" > .contribs
sed -i '/@CONTRIBUTORS@/r .contribs' $FILE 
sed -i '/@CONTRIBUTORS@/d' $FILE 
rm .contribs

echo "
A release notes template is available in release_notes.rst. You still need
to update it with features, etc.!  
"
