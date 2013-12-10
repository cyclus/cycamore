#! /usr/bin/python

from nose.tools import assert_not_equal
import sys
import os
import tables
import numpy as np
from tools import check_cmd

""" Tests """
def test_null_sink():
    #Cyclus simulation input for null sink testing
    sim_input = "./inputs/null_sink.xml"
    holdsrtn = [1] # needed because nose does not send() to test generator
    cmd = ["cyclus", "-o", "./output_temp.h5", "--input-file", sim_input]
    check_cmd(cmd, '.', holdsrtn)
    rtn = holdsrtn[0]
    if rtn != 0:
        return # don't execute further commands

    output = tables.open_file("./output_temp.h5", mode = "r")
    paths = [] # this must contain tables to test
    for node in output.walkNodes(classname = "Table"):
        paths.append(node._v_pathname)
    
    for path in paths:
        #No resource exchange is expected
        yield assert_not_equal, path, "/Transactions"

    output.close()
    os.remove("./output_temp.h5")
