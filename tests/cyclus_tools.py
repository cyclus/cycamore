import os
from tools import check_cmd

from numpy import array_equal
import tables

import visitors

def run_cyclus(cyclus, cwd, in_path, out_path):
    """Runs cyclus with various inputs and creates output databases
    """
    holdsrtn = [1]  # needed because nose does not send() to test generator
    # make sure the output target directory exists
    cmd = [cyclus, "-o", out_path, "--input-file", in_path]
    check_cmd(cmd, cwd, holdsrtn)

def db_comparator(path1, path2):
    """Compares two Cyclus HDF5 databases 

    Returns:
            True or False. In case of False, it prints out the names
            and differences in the compared databases.
    """
    v1 = visitors.HDF5RegressionVisitor(path1)
    v2 = visitors.HDF5RegressionVisitor(path2)
    return v1.walk() == v2.walk()
