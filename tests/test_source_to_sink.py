#! /usr/bin/python

from nose.tools import assert_equal
from numpy.testing import assert_array_equal, assert_array_almost_equal
import sys
import subprocess
import tables
import numpy as np
from tools import check_cmd

""" Tests """
def test_source_to_sink():
    #Cyclus simulation inputs
    sim_inputs = ["./inputs/source_to_sink.xml"]

    for sim_input,bench_db in zip(sim_inputs,bench_dbs):
        holdsrtn = [1] # needed because nose does not send() to test generator
        cmd = ["cyclus", "-o", "./output_temp.h5", "--input-file", sim_input]
        yield check_cmd, cmd, '.', holdsrtn
        rtn = holdsrtn[0]
        if rtn != 0:
            return # don't execute further commands

        bench = tables.open_file(bench_db, mode = "r")
        output = tables.open_file("./output_temp.h5", mode = "r")
        paths = []
        # Test certain behaviour of the output on these paths
        # Linear accumulation of resources in the sink facility
