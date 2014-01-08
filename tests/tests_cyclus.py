#! /usr/bin/env python

import os

from tests_list import sim_files
from cyclus_tools import run_cyclus, db_comparator

"""Tests"""
def test_cyclus():
    """Test for all inputs in sim_files. Checks if reference and current cyclus 
    output is the same.

    WARNING: the tests require cyclus executable to be included in PATH
    """
    cwd = os.getcwd()
    
    for sim_input,bench_db in sim_files:

        temp_output = [(sim_input, "./output_temp.h5")]
        yield run_cyclus("cyclus", cwd, temp_output)

        yield db_comparator(bench_db, "./output_temp.h5")

        os.remove("./output_temp.h5")
