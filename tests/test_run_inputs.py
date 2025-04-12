import os
import subprocess

from pytest import skip
import pytest

import run_inputs as ri
from helper import cyclus_has_coin


def coin_skipper(filename):
    raise skip(filename + " cannot be executed since Cyclus was not installed "
                   "with COIN support")

def get_files():
    files, _, _ = ri.get_files(ri.input_path)
    for f in files:
        yield f

@pytest.fixture(params=get_files())
def file_fixture(request):
    return request.param

def test_inputs(file_fixture):
    absfile = os.path.join(ri.input_path, file_fixture)
    with open(absfile) as fh:
        src = fh.read()
    if cyclus_has_coin() or "GrowthRegion" not in src:
        testf = ri.TestFile(ri.cyclus_path, file_fixture, "-v0")
        testf.run()
        assert testf.passed, "Failed running {}".format(file_fixture)
    else:
        coin_skipper(absfile)
