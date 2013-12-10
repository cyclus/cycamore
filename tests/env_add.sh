#! /usr/bin/bash

#Adds PyTables 3 built in my directory on CAE
#Updates PYTHONPATH, LD_LIB_PATH variables

export PYTHONPATH=$PYTHONPATH:$CNERG_ROOT/users/rakhimov/install/lib/python2.7/site-packages
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CNERG_ROOT/users/rakhimov/hdf5-1.8.11/hdf5/lib

