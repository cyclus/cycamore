#! /usr/bin/bash

#Adds PyTables 3 built in my directory on CAE
#Updates PYTHONPATH, LD_LIB_PATH variables

export PYTHONPATH=$PYTHONPATH:$CNERG_ROOT/users/rakhimov/install/lib/python2.7/site-packages
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CNERG_ROOT/users/rakhimov/hdf5-1.8.11/hdf5/lib

# For some unkown reasons sphinx 1.1.3 libraries for python 2.6 breaks pytables
# Remove that libraries from pythonpath

export PYTHONPATH=`echo $PYTHONPATH | sed 's/\/filespace\/groups\/cnerg\/opt\/Sphinx-1.1.3\/lib\/python2.6\/site-packages\///g'`

