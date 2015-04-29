#!/bin/bash

set -x
set -e

die() {
    echo >&2 "$@"
    exit 1
}

# check input
CORE=${CORE_DIR?"Environment variable CORE_DIR must be set to the cyclus repository directory."}
CYCAMORE=${CYCAMORE_DIR?"Environment variable CYCAMORE_DIR must be set to the cycamore repository directory."}
[ "$#" -eq 1 ] || die "Must provide the version (e.g., X.X.X) as an argument" 
VERSION=$1
echo "Conda updating for Cyclus stack verison $VERSION"

mkdir _build
cd _build
export CONDA="$PWD/miniconda"
export PATH="$CONDA/bin:$PATH"
export PYTHONPATH="$CONDA"
wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O miniconda.sh;
bash miniconda.sh -b -p $CONDA
hash -r
conda config --set always_yes yes --set changeps1 no
conda config --add channels pyne
conda config --add channels cyclus
conda update -q conda
conda install conda-build jinja2 setuptools binstar patchelf nose
# Useful for debugging any issues with conda
conda info -a
cd ..

cd $CORE
cp conda-recipe/meta.yaml conda-recipe/.orig.meta.yaml
sed -i  "s/version: 0.0/version: $VERSION/g" conda-recipe/meta.yaml
sed -i  "s/string: nightly/string: 0/g" conda-recipe/meta.yaml
conda build --no-test conda-recipe
binstar upload --force -u cyclus $CONDA/conda-bld/linux-64/cyclus-$VERSION-0.tar.bz2
mv conda-recipe/.orig.meta.yaml conda-recipe/meta.yaml

cd $CYCAMORE
cp conda-recipe/meta.yaml conda-recipe/.orig.meta.yaml
sed -i  "s/version: 0.0/version: $VERSION/g" conda-recipe/meta.yaml
sed -i  "s/string: nightly/string: 0/g" conda-recipe/meta.yaml
conda build --no-test conda-recipe
binstar upload --force -u cyclus $CONDA/conda-bld/linux-64/cycamore-$VERSION-0.tar.bz2
mv conda-recipe/.orig.meta.yaml conda-recipe/meta.yaml
