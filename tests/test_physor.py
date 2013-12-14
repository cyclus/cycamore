#! /usr/bin/env python

from numpy.testing import assert_array_equal
import os
import tables
import numpy as np
from tools import check_cmd

"""Tests"""
def test_physor():
    """Test for physor cases. Checks if reference and current cyclus physor
    output is the same. 

    WARNING: the tests require either alias cyclus='location of cyclus'
             or the location of cyclus executable to be included in PATH
    """

    # Cyclus simulation inputs
    sim_inputs = ["./inputs/physor/1_Enrichment_2_Reactor.xml",
                  "./inputs/physor/2_Sources_3_Reactors.xml"]
    # Benchmark databases must match the order of input files
    bench_dbs = ["./benchmarks/physor_1_Enrichment_2_Reactor.h5",
                  "./benchmarks/physor_2_Sources_3_Reactors.h5"]

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
        for node in bench.walk_nodes(classname = "Table"):
            paths.append(node._v_pathname)
       
        for path in paths:
            bdata = bench.get_node(path)[:]
            odata = output.get_node(path)[:]
            names = []
            for name in bdata.dtype.names:
                if name != "SimID":
                    names.append(name)
            bdata = bdata[names]
            odata = odata[names]
            yield assert_array_equal, bdata, odata
        
        bench.close()
        output.close()
        os.remove("./output_temp.h5")
