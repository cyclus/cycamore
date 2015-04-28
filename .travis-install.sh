#!/bin/bash

set -x # print cmds
set -e # exit as soon as an error occurs

# log
msg=`git log --pretty=oneline -1`
echo "Building commit: $msg" 

# setup conda recipe to use develop cyclus
sed -i  "s/- cyclus/- cyclus 0.0/g" conda/meta.yaml

# build
conda build --no-test conda

# install
conda install --use-local cycamore=0.0
