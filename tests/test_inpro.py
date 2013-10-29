#! /usr/bin/python

from nose.tools import assert_equal
from numpy.testing import assert_array_equal, assert_array_almost_equal
import sys
import subprocess
import tables
import numpy as np
from tools import check_cmd



def h5_comparator(file_one, file_two):
    """ Compares two hdf5 outputs of Cyclus.
    
    Returns:
        Boolean: True if files are equal
    
    Raises:
        IOError: An error accessing hdf5 files
        SystemExit: Carries messages for various errors and False results 
    """
    try:
            if not tables.isHDF5File(file_one) :
                
                sys.exit(file_one + " is not a hdf5 file.")
                
            if not tables.isHDF5File(file_two) is not True:
                
                sys.exit(file_two + " is not a hdf5 file.")
        
    except tables.HDF5ExtError:
        sys.exit("Error at input file validation:" + str(sys.exc_info()[0]))
        

    db_one = tables.openFile(file_one, mode = "r")
    db_two = tables.openFile(file_two, mode = "r")
    
    """ Comparing tow databases """
    table_names_one = db_one.getNode("/").__members__
    table_names_two = db_two.getNode("/").__members__
    
    if table_names_one == table_names_two:
        for table_name in table_names_one:
            # "InputFiles" table is omitted for now
            if table_name != "InputFiles":
                table_one = db_one.getNode("/", name = table_name)
                table_two = db_two.getNode("/", name = table_name)
                table_one_columns = table_one.colnames
                table_two_columns = table_two.colnames
                
                if table_one_columns == table_two_columns :
                    for column in table_one_columns:
                        # Avoiding SimID comparison
                        if column != "SimID":
                            if table_one.col(column)[0] == table_two.col(column)[0]:
                                continue
                            else:
                                sys.exit("The Difference is in "+ column +" of "
                                          + table_name )
                    
                else:
                    sys.exit("The column names or order does not match in " +
                             table_name)
    else:
        sys.exit("The files do not have the same groups, group ordering or naming.")
            
    db_one.close()
    db_two.close()
    
    return True
    

""" Tests """
def test_inpro_low():
    # Calling Cyclus with inpro_low input
    holdsrtn = [1] # needed because nose does not send() to test generator
    cmd = ["cyclus", "-o", "./outputs/output_inpro_low.h5", "--input-file", "./inputs/inpro_low.xml"]
    yield check_cmd, cmd, '.', holdsrtn
    rtn = holdsrtn[0]
    if rtn != 0:
        return # don't execute further commands
# h5_comparator("./benchmarks/output_inpro_low_bench.h5", 
#                "./outputs/output_inpro_low.h5")

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


#def test_inpro_high():
    # Calling Cyclus with inpro_high input
#   call_cyclus("./inputs/inpro_low.xml","./outputs/output_inpro_low.h5")
#    h5_comparator("./benchmarks/output_inpro_low_bench.h5", 
#                "./outputs/output_inpro_low.h5")

