PyTables 3.0.0 or higher is required for tests.
cyclus executable is expected to be included in PATH.
Reference databases must be located in benchmarks directory

The expected workflow for a user is 
    1) Run 'create_references.py' if there are no reference databases or 
        they need to be updated.
    2) Run 'nosetests test_cyclus.py'
