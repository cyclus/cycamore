#!/bin/bash

set -e

die() {
    echo >&2 "$@"
    exit 1
}

HERE=$PWD

# check input
CORE=${CORE_DIR?"Environment variable CORE_DIR must be set to the cyclus repository directory."}
CYCA=${CYCAMORE_DIR?"Environment variable CYCAMORE_DIR must be set to the cycamore repository directory."}
[ "$#" -eq 2 ] || die "Input requires from version and to version, e.g., 
$ ./make_release_notes.sh 0.2.0 0.3.0
" 
PREV=$1
VERSION=$2
echo "Making release notes template for Cyclus stack verison $VERSION from $PREV. 

Note, these version names must refer to *existing tags* in all repositories.
"

# cyclus summary
cd $CORE
NCOMMITS=`git rev-list $PREV...$VERSION --count | tail -n1`
SUMMARY=`git diff --stat $PREV...$VERSION | tail -n1`
CORETXT="$NCOMMITS commits resulting in $SUMMARY"
CORECONTRIB=`git log --format='%aN' $PREV...$VERSION | sort -u`
cd $HERE

# cycamore summary
cd $CYCA
NCOMMITS=`git rev-list $PREV...$VERSION --count | tail -n1`
SUMMARY=`git diff --stat $PREV...$VERSION | tail -n1`
CYCATXT="$NCOMMITS commits resulting in $SUMMARY"
CYCACONTRIB=`git log --format="%aN" $PREV...$VERSION | sort -u`
cd $HERE

# contributors, beware, thar be hackery ahead
echo "Raw core contributors:"
echo "$CORECONTRIB"
echo ""
echo "$CORECONTRIB" > .contribs
echo "Raw cyca contributors:"
echo "$CYCACONTRIB"
echo ""
echo "$CYCACONTRIB" >> .contribs
CONTRIBTXT=`cat .contribs | sort -u | awk '{print "* " $0}'`
echo "$CONTRIBTXT" > .contribs

# replace
FILE=release_notes.rst
cp -i release_notes.rst.in $FILE
sed -i "s/@PREV_VERSION@/$PREV/g" $FILE 
sed -i "s/@VERSION@/$VERSION/g" $FILE 
sed -i "s/@CORE_SUMMARY@/$CORETXT/g" $FILE 
sed -i "s/@CYCA_SUMMARY@/$CYCATXT/g" $FILE 
sed -i '/@CONTRIBUTORS@/r .contribs' $FILE 
sed -i '/@CONTRIBUTORS@/d' $FILE 
rm .contribs

echo "
A release notes template is available in release_notes.rst. You still need
to update it with features, etc.!  
"
