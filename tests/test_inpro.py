#! /usr/bin/python

from nose.tools import assert_equal
from numpy.testing import assert_array_equal, assert_array_almost_equal
import sys
import subprocess
import tables
import numpy as np
from tools import check_cmd

""" Tests """
def test_inpro_low():
    # Calling Cyclus with inpro_low input
    holdsrtn = [1] # needed because nose does not send() to test generator
    cmd = ["cyclus", "-o", "./outputs/output_inpro_low.h5", "--input-file", "./inputs/inpro_low.xml"]
    yield check_cmd, cmd, '.', holdsrtn
    rtn = holdsrtn[0]
    if rtn != 0:
        return # don't execute further commands

    bench = tables.open_file("./benchmarks/bench_inpro_low.h5", mode = "r")
    output = tables.open_file("./outputs/output_inpro_low.h5", mode = "r")
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

