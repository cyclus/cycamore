import os
import subprocess

from nose.tools import assert_true

import run_inputs as ri

def test_inputs():
    files, _, _ = ri.get_files(ri.input_path)
    for f in files:
        testf = ri.TestFile(ri.cyclus_path, f, "-v0") 
        testf.run()
        yield assert_true, testf.passed
