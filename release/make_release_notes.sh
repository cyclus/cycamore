FILE=release_notes.rst
cp release_notes.rst.in $FILE

HERE=$PWD
CORE=~/work/cyclus/cyclus
CYCA=~/work/cyclus/cycamore

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
sed -i '/@CONTRIBUTORS@/r contribs' $FILE 
sed -i '/@CONTRIBUTORS@/d' $FILE 
rm .contribs
