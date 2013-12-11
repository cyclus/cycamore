#! /usr/bin/python

from nose.tools import assert_equal
from numpy.testing import assert_array_equal, assert_array_almost_equal
import sys
import os
import subprocess
import tables
import numpy as np
from tools import check_cmd

""" Tests """
def test_source_to_sink():
    #Cyclus simulation input for source_to_sink
    sim_input = "./inputs/source_to_sink.xml"

    holdsrtn = [1] # needed because nose does not send() to test generator
    cmd = ["cyclus", "-o", "./output_temp.h5", "--input-file", sim_input]
    check_cmd(cmd, '.', holdsrtn)
    rtn = holdsrtn[0]
    if rtn != 0:
        return # don't execute further commands

    output = tables.open_file("./output_temp.h5", mode = "r")
    paths = []
    for node in output.walkNodes(classname = "Table"):
        paths.append(node._v_pathname)
    # Test certain behaviour of the output on these paths
    # Linear accumulation of resources in the sink facility

    output.close()
    os.remove("./output_temp.h5")
