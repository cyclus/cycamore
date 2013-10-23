#! /usr/bin/python

import sys
import subprocess
import tables


def h5_comparator(file_one, file_two):
    """ Compares two hdf5 outputs of Cyclus.
    
    Returns:
        Boolean: True if files are equal
    
    Raises:
        IOError: An error accessing hdf5 files
        SystemExit: Carries messages for various errors and False results 
    """
    try:
            if (tables.isHDF5File(file_one) is not True):
                
                sys.exit(file_one + " is not a hdf5 file.")
                
            if (tables.isHDF5File(file_two) is not True):
                
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
    

def call_cyclus(input_file, output_file,timer = True):
    """ Runs Cyclus with a given input file """

    tm = "time" if timer else ""

    subprocess.call([tm,"cyclus", "-o",output_file,
       "--input-file", input_file])
   



""" Running the temporary checking code."""


if __name__ == "__main__":
    try:
        if ( len(sys.argv) < 3 ):
            sys.exit("Provide a correct call with 2 files to compare.")
            
        if h5_comparator(sys.argv[1],sys.argv[2]) is True:
            print( sys.argv[1] + " has identical values as " + sys.argv[2])
        
    except SystemExit as err:
        print(err)
    
""" Tests """
def test_inpro_low():
    # Calling Cyclus with inpro_low input
    call_cyclus("./inputs/inpro_low.xml","./outputs/output_inpro_low.h5")
    h5_comparator("./benchmarks/output_inpro_low_bench.h5", 
                "./outputs/output_inpro_low.h5")

#def test_inpro_high():
    # Calling Cyclus with inpro_high input
#   call_cyclus("./inputs/inpro_low.xml","./outputs/output_inpro_low.h5")
#    h5_comparator("./benchmarks/output_inpro_low_bench.h5", 
#                "./outputs/output_inpro_low.h5")

