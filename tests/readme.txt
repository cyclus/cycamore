Benchmarks for inpro cases must be located in "./benchmarks" directory.
Assumes cyclus can be successfully built with all dependencies provided.
Assumes all tests run successfully and run_input.py is successful.
PyTables 3.0.0 or higher is required to run tests.
"output_temp.h5" will be created and deleted by tests in the directory.
For CAE machines, run env_add.sh script to switch to PyTables 3.
env_add.sh deletes cnerg.env specific sphinx path from PYTHONPATH
