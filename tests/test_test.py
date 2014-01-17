
import os
from nose.tools import assert_true

import visitors

from testcases import sim_files
from cyclus_tools import run_cyclus

def db_comparator(path1, path2):
    v1 = visitors.HDF5RegressionVisitor(path1)
    v2 = visitors.HDF5RegressionVisitor(path2)
    return v1.walk() == v2.walk()

def test_cyclus():
    
    for sim_input, bench_db in sim_files:
        
        print("testing input: " + sim_input + " and bench_db: " + bench_db)
        tmp_file = "tmp.h5"

        temp_output = [(sim_input, tmp_file)]
        run_cyclus("cyclus", os.getcwd(), temp_output)

        if os.path.isfile(tmp_file):
            assert_true(db_comparator(bench_db, tmp_file))
            os.remove(tmp_file)
