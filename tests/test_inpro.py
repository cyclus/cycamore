#! /usr/bin/python

from nose.tools import assert_equal
from numpy.testing import assert_array_equal, assert_array_almost_equal
import sys
import subprocess
import tables
import numpy as np
from tools import check_cmd

def comparator(sim_input, bench_db):
    # Calling Cyclus with inpro_low input
    holdsrtn = [1] # needed because nose does not send() to test generator
    cmd = ["cyclus", "-o", "./outputs/output_temp.h5", "--input-file", sim_input]
    yield check_cmd, cmd, '.', holdsrtn
    rtn = holdsrtn[0]
    if rtn != 0:
        print ( "returning!")
        return # don't execute further commands

    bench = tables.open_file(bench_db, mode = "r")
    output = tables.open_file("./outputs/output_temp.h5", mode = "r")
    paths = []
    for node in bench.walkNodes(classname = "Table"):
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

""" Tests """
def test_inpro():
        #Cyclus simulation inputs
    sim_inputs = [#"./inputs/inpro_low.xml",
                  #"./inputs/inpro_high.xml", 
                  "./inputs/inpro_low_short.xml",
                  "./inputs/inpro_low_no_decay.xml",
                  "./inputs/inpro_high_no_dacay.xml"]
    #Benchmark databases must match the order of input files
    bench_dbs = [#"./benchmarks/bench_inpro_low.h5",
                 #"./benchmarks/bench_inpro_high.h5",
                 "./benchmarks/bench_inpro_low_short.h5",
                 "./benchmarks/bench_inpro_low_no_decay.h5",
                 "./benchmarks/bench_inpro_high_no_decay.h5"]

    for sim_input,bench_db in zip(sim_inputs,bench_dbs):
        # Calling Cyclus with inpro_low input
        holdsrtn = [1] # needed because nose does not send() to test generator
        cmd = ["cyclus", "-o", "./outputs/output_temp.h5", "--input-file", sim_input]
        yield check_cmd, cmd, '.', holdsrtn
        rtn = holdsrtn[0]
        if rtn != 0:
            print ( "returning!")
            return # don't execute further commands

        bench = tables.open_file(bench_db, mode = "r")
        output = tables.open_file("./outputs/output_temp.h5", mode = "r")
        paths = []
        for node in bench.walkNodes(classname = "Table"):
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

def test_inpro_low_no_decay():
    # Calling Cyclus with inpro_low input
    holdsrtn = [1] # needed because nose does not send() to test generator
    cmd = ["cyclus", "-o", "./outputs/output_temp.h5", "--input-file", 
            "./inputs/inpro_low_no_decay.xml"]
    yield check_cmd, cmd, '.', holdsrtn
    rtn = holdsrtn[0]
    if rtn != 0:
        print ( "returning!")
        return # don't execute further commands

    bench = tables.open_file("./benchmarks/bench_inpro_low_no_decay.h5",
                            mode = "r")
    output = tables.open_file("./outputs/output_temp.h5", mode = "r")
    paths = []
    for node in bench.walkNodes(classname = "Table"):
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
