import os
from tools import check_cmd

import numpy as np
import tables

import visitors

def run_cyclus(cyclus, cwd, in_path, out_path):
    """Runs cyclus with various inputs and creates output databases
    """
    holdsrtn = [1]  # needed because nose does not send() to test generator
    # make sure the output target directory exists
    cmd = [cyclus, "-o", out_path, "--input-file", in_path]
    check_cmd(cmd, cwd, holdsrtn)

def compare_nondeterm(path1, path2):
    """Compares two Cyclus HDF5 databases assuming non-deterministic AgentIDs
    and TransactionIDs

    Returns:
            True or False. 
    """
    v1 = visitors.HDF5RegressionVisitor(path1)
    v2 = visitors.HDF5RegressionVisitor(path2)
    return v1.walk() == v2.walk()

def compare_determ(path1, path2, verbose=False):
    """Compares two Cyclus HDF5 databases assuming deterministic AgentIDs and
    TransactionIDs

    Returns:
            True or False. In case of False, it prints out the names
            and differences in the compared databases.
    """
    dbs_same = True
    db_one = tables.open_file(path1, mode = "r")
    db_two = tables.open_file(path2, mode = "r")
    path_one = []
    path_two = []

    for node in db_one.walk_nodes(classname = "Table"):
        path_one.append(node._v_pathname)

    for node in db_two.walk_nodes(classname = "Table"):
        path_two.append(node._v_pathname)
    
    # Check if databases contain the same tables
    if not np.all(path_one == path_two):
        if verbose:
            print("The number or names of tables in databases are not the same.")
            print(path_one)
            print(path_two)
        # Close databases
        db_one.close()
        db_two.close()
        dbs_same = False
        return dbs_same

    paths = path_one

    for path in paths:
        data_one = db_one.get_node(path)[:]
        data_two = db_two.get_node(path)[:]
        names = []

        for name in data_one.dtype.names:
            if name != "SimID":
                names.append(name)

        data_one = data_one[names]
        data_two = data_two[names]

        if not np.all(data_one == data_two):
            dbs_same = False
            if verbose:
                print("\n" + path + " table is different in the databases.")
                # Investigation of the differences
                # check if the lengths are different
                if len(data_one) != len(data_two):
                    print("Length mismatch: " + len(data_one) + ", " + len(data_two))
                else:
                    for name in names:
                        column_one = data_one[name]
                        column_two = data_two[name]
                        # check if data types are the same
                        if column_one.dtype != column_two.dtype:
                            print("Datatypes in column " + name +" are different.")
                            print(column_one.dtype)
                            print(column_two.dtype)
                        elif not np.array_equal(column_one, column_two):
                            print("The difference is in column " + name)
                            diff = np.equal(column_one, column_two)
                            # find indices and elements for numerical values
                            indices = np.where(diff==False)
                            # check if whole table is different
                            if len(indices) == len(column_one):
                                print("All the elements in this column are different")
                            else:
                                # provide mismatch percentage
                                mismatch = 100*float(len(indices))/len(column_one)
                                print("Mismatch is " + str(mismatch) + "%")
                                print("Indices of different objects are ")
                                print(indices[0]) # this prints indices in a clearer way
                                print("The different elements on these indices.")
                                print(column_one[indices])
                                print(column_two[indices])
                                print("") # a new line to make reading the output easier

    # Close databases
    db_one.close()
    db_two.close()
    return dbs_same
