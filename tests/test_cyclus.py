#! /usr/bin/env python

import os
from nose.tools import assert_true

from testcases import sim_files
from cyclus_tools import run_cyclus, db_comparator

def test_cyclus():
    """Test for all inputs in sim_files. Checks if reference and current cyclus 
    output is the same.

    WARNING: the tests require cyclus executable to be included in PATH
    """    
    for sim_input, bench_db in sim_files:
        
        # print("testing input: " + sim_input + " and bench_db: " + bench_db)
        tmp_file = "tmp.h5"
        
        run_cyclus("cyclus", os.getcwd(), sim_input, tmp_file)
        
        if os.path.isfile(tmp_file):
            assert_true(db_comparator(bench_db, tmp_file))
            os.remove(tmp_file)
