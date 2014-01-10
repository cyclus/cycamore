
import os

import sys

import tables
import numpy as np
from numpy.testing import assert_array_equal

from tools import check_cmd

def run_cyclus(cyclus, cwd, sim_files):
    """Runs cyclus with various inputs and creates output databases
       """

    for sim_input, sim_output in sim_files:
        holdsrtn = [1]  # needed because nose does not send() to test generator
        # make sure the output target directory exists
        if not os.path.exists(os.path.dirname(sim_output)):
            os.makedirs(os.path.dirname(sim_output))

        cmd = [cyclus, "-o", sim_output, "--input-file", sim_input]
        check_cmd(cmd, cwd, holdsrtn)
        rtn = holdsrtn[0]
        if rtn != 0:
            return  # don"t execute further commands

def db_comparator(file_one, file_two):
    """Compares two hdf5 databases
    
    Returns:
            True or False. In case of False, it prints out the names
            and differences in the compared databases.
    """

    dbs_same = True
    db_one = tables.open_file(file_one, mode = "r")
    db_two = tables.open_file(file_two, mode = "r")
    path_one = []
    path_two = []

    for node in db_one.walk_nodes(classname = "Table"):
        path_one.append(node._v_pathname)

    for node in db_two.walk_nodes(classname = "Table"):
        path_two.append(node._v_pathname)

    # Check if databases contain the same tables
    try:
        assert_array_equal(path_one, path_two)
        paths = path_one
    except AssertionError as err:
        print("The number or names of tables in databases are not the same.")
        print(err.message)
        # Close databases
        db_one.close()
        db_two.close()
        dbs_same = False
        return dbs_same

    for path in paths:
        data_one = db_one.get_node(path)[:]
        data_two = db_two.get_node(path)[:]
        names = []
        for name in data_one.dtype.names:
            if name != "SimID":
                names.append(name)
        data_one = data_one[names]
        data_two = data_two[names]
        try:
            assert_array_equal(data_one, data_two)
        except AssertionError as err:
            print("\n" + path + " table are different in the databases.")
            print(err.message)
            dbs_same = False
    # Close databases
    db_one.close()
    db_two.close()
    return dbs_same
