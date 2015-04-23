#!/bin/bash

set -x # print cmds
set -e # exit as soon as an error occurs

# log
msg=`git log --pretty=oneline -1`
echo "Building commit: $msg" 

# setup conda recipe
wget https://github.com/gidden/ciclus/archive/travis.zip -O ciclus.zip
unzip -j ciclus.zip "*/cycamore/*" -d conda-recipe
sed -i  "s/- cyclus/- cyclus 0.0/g" conda-recipe/meta.yaml

# build
conda build --no-test conda-recipe

# install
conda install --use-local cycamore=0.0
