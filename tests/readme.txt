PyTables 3.0.0 or higher is required for tests.
cyclus executable is expected to be included in PATH.

The expected workflow for a user is 
    1) Run 'create_references.py' if there are no reference databases or 
        they need to be updated.
    2) Run 'nosetests test_cyclus.py'
    3) If more detailed report is needed about differences between particular
        hdf5 databases, the user has to write a script importing
        db_comparator function from cyclus_tools.py.
