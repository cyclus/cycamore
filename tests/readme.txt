PyTables 3.0.0 or higher is required for tests.
cyclus executable is expected to be included in PATH.
Reference databases must be located in benchmarks directory

The expected workflow for a user is 
    1) Run 'create_references.py' if there are no reference databases or 
        they need to be updated.
    2) Run 'nosetests test_cyclus.py'

The following bash commands accomplish the workflow given above.
Notation:
    '#' at the start of a line indicates a comment line.
    '$' at the start of a line indicates a command.
Script:
# These commands can be run from CLI or as a bash script
# These commands are convenient to execute inside /PATH_TO_CYCAMORE/tests/
# directory, but it is not a requirement.

# Moving to tests directory

$ cd /PATH_TO_CYCAMORE/tests/

# Assuming cyclus executable is in PATH
# Creating reference databases in benchmarks directory for simulation input
# and output files specified in testcases.py file. If references already exist
# in benchmarks directory, they will be OVERWRITTEN.
# Thus, skip this step if references exist and do not need to be recreated.

$ python ./create_references.py

# Running nosetest

$ nosetests ./test_cyclus.py

# The number of failures may not be equal to the number of databases.
# The failures need to be investigated with other tools, like vitables.

