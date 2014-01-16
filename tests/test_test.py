
import os
from nose.tools import assert_true

import tables

import visitors

from testcases import sim_files
from cyclus_tools import run_cyclus

def db_comparator(path1, path2):
    db1 = tables.open_file(path1, mode = "r")
    db2 = tables.open_file(path2, mode = "r")
    v1 = visitors.HDF5RegressionVisitor(db1)
    v2 = visitors.HDF5RegressionVisitor(db2)
    obj1 = v1.walk()
    obj2 = v2.walk()

    thing1 = obj1.pop()
    thing2 = obj2.pop()
    print([thing1[i][2] for i in range(len(thing1))])
    print([thing2[i][2] for i in range(len(thing2))])
    # print(str(len(obj1)) + " " + str(len(obj2)))
    # print(obj1)
    # print(obj2)
    return obj1 == obj2

def test_cyclus():
    
    for sim_input, bench_db in sim_files:
        
        print("testing input: " + sim_input + " and bench_db: " + bench_db)

        temp_output = [(sim_input, "./output_temp.h5")]
        yield run_cyclus, "cyclus", os.getcwd(), temp_output

        if os.path.isfile("./output_temp.h5"):
            yield assert_true, db_comparator(bench_db, "./output_temp.h5")
            os.remove("./output_temp.h5")

if __name__ == "__main__":
    test_cyclus()
