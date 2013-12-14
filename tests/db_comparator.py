from numpy import array_equal
import sys
import tables
import numpy as np

def main(argv):
    """ Compare two databases

    Arguments: must be two databases

    Returns: message that indicates if the databases differ
    """
    # Check for valid arguments
    if (len(argv) != 3):
        print("Takes exactly two arguments - locations of databases")
        sys.exit()

     # Keep track of opened databases
    db_one_opened = False
    db_two_opened = False

    # Success state
    dbs_same = True

    try:
        db_one = tables.open_file(argv[1], mode = "r")
        db_one_opened = True
        db_two = tables.open_file(argv[2], mode = "r")
        db_two_opened = True
        path_one = []
        path_two = []

        for node in db_one.walk_nodes(classname = "Table"):
            path_one.append(node._v_pathname)

        for node in db_two.walk_nodes(classname = "Table"):
            path_two.append(node._v_pathname)

        # Check if databases contain the same tables
        if array_equal(path_one, path_two):
            paths = path_one
        else:
            print("The number or names of tables in databases are not the same.")
            sys.exit()

        for path in paths:
            data_one = db_one.get_node(path)[:]
            data_two = db_two.get_node(path)[:]
            names = []
            for name in data_one.dtype.names:
                if name != "SimID" and name != "InputFiles":
                    names.append(name)
            data_one = data_one[names]
            data_two = data_two[names]
            if (not array_equal(data_one, data_two)):
                print(name + " table are different in the databases.")
                dbs_same = False

        if dbs_same:
            print("The databases are the same.")

    finally:
        # Close opened databases
        if db_one_opened: db_one.close()
        if db_two_opened: db_two.close()

if __name__ == "__main__": main(sys.argv)
