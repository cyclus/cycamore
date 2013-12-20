#! /usr/bin/env python

import sys
import argparse as ap

import tables
import numpy as np
from numpy.testing import assert_array_equal

def main():
    """Compare two databases

    Arguments: must be two hdf5 databases

    Returns: message that indicates if the databases differ

    Raises: IOError if the databases don't exist
    """
    description = "Compares two hdf5 databases."
    parser = ap.ArgumentParser(description=description)

    file_one = "First database"
    parser.add_argument("file_one", help=file_one)

    file_two = "Second database"
    parser.add_argument("file_two", help=file_two)
    
    args = parser.parse_args()

    # Keep track of opened databases
    db_one_open = False
    db_two_open = False


    try:

        main_body(args, db_one_open, db_two_open)

    finally:
        # Close opened databases
        if db_one_open: db_one.close()
        if db_two_open: db_two.close()

def main_body(args, db_one_open, db_two_open):
    """The body of main that compares two hdf5 databases"""
    # Success state
    dbs_same = True

    db_one = tables.open_file(args.file_one, mode = "r")
    db_one_open = True
    db_two = tables.open_file(args.file_two, mode = "r")
    db_two_open = True
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
        sys.exit()

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

    if dbs_same:
        print("The databases are the same.")

if __name__ == "__main__": main()
