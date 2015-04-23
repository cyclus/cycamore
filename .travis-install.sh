#!/bin/bash

# log
msg=`git log --pretty=oneline -1`
echo "Building commit: $msg" 

# setup conda recipe
wget https://github.com/gidden/ciclus/archive/travis.zip -O ciclus.zip
unzip -j ciclus.zip "*/cycamore/*" -d conda-recipe
sed -i  "s/- cyclus/- cyclus 0.0/g" conda-recipe/meta.yaml

# build
cmd="conda build --no-test conda-recipe"
echo "cmd: $cmd"
$cmd
status=$?
echo "status: $status"
if [[ $status != 0 ]]; then
    exit $status
fi

# install
cmd="conda install --use-local cycamore=0.0"
echo "cmd: $cmd"
$cmd
status=$?
echo "status: $status"
if [[ $status != 0 ]]; then
    exit $status
fi
