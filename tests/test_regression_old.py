#! /usr/bin/env python

import os
import json
import hashlib
import urllib
import uuid
from nose.tools import assert_true
from nose import with_setup

from cyclus_tools import run_cyclus, compare_determ, compare_nondeterm

sim_files = {}
fetchdir = "fetch"

def setup():
    global sim_files
    if not os.path.isdir(fetchdir): 
        os.makedirs(fetchdir)
    with open("reflist.json") as f:
        refs = json.load(f)
    cyclus_ref = refs[-1]["cyclus-ref"]
    cycamore_ref = refs[-1]["cycamore-ref"]
    refs = [r for r in refs 
            if r["cyclus-ref"] == cyclus_ref 
            and r["cycamore-ref"] == cycamore_ref]
    base_url = "http://regtests.fuelcycle.org/"
    for r in refs:
        fpath = os.path.join(fetchdir, r["fname"])
        if not os.path.exists(fpath):
            try:
                urllib.urlretrieve(base_url+r["fname"], fpath)
            except AttributeError: # try python 3.1+ api version
                urllib.request.urlretrieve(base_url+r["fname"], fpath)
        h = hashlib.sha1()
        with open(fpath, "rb") as f: 
            h.update(f.read())
        if h.hexdigest() != r["sha1-checksum"]:
            raise RuntimeError("They tooks our data!!! All our rackspace are belong to them.")
        sim_files[r["input-file"]] = fpath

class TestRegression(object):
    def __init__(self):
        self.in_dir_ = "../input"
        self.tmp_files_ = {}        
        for root, dirs, files in os.walk(self.in_dir_):
            for f in files:
                self.tmp_files_[f] = str(uuid.uuid4()) + ".h5"

    def __del__(self):
        for inf, outf in self.tmp_files_.items():
            if os.path.isfile(outf):
                print("removing {0}".format(outf))
                os.remove(outf)
                
    def teardown(self):
        for inf, outf in self.tmp_files_.items():
            if os.path.isfile(outf):
                print("removing {0}".format(outf))
                os.remove(outf)

    def test_regression(self, check_deterministic=False):
        """Test for all inputs in sim_files. Checks if reference and current cyclus 
        output is the same.

        Parameters
        ----------
        check_deterministic : bool
            If True, also test determinisitc equality of simulations
        
        WARNING: the tests require cyclus executable to be included in PATH
        """
        for root, dirs, files in os.walk(self.in_dir_):
            for f in files:
                if not f.endswith('.xml'):
                    continue
                tmp_file = self.tmp_files_[f]
                run_cyclus("cyclus", os.getcwd(), os.path.join(root, f), 
                           tmp_file)

                if os.path.isfile(tmp_file):
                    if f not in sim_files:
                        continue # nada to do, just making sure it runs
                    if check_deterministic:
                        determ = compare_determ(sim_files[f], tmp_file, 
                                                verbose=True)
                        assert_true(determ)
                    else:         
                        nondeterm = compare_nondeterm(sim_files[f], tmp_file)
                        assert_true(nondeterm)

                if os.path.isfile(tmp_file):
                    print("removing {0}".format(tmp_file))
                    os.remove(tmp_file)
                
                tmp_file = tmp_file.split('.')[0] + '.sqlite'
                
                if os.path.isfile(tmp_file):
                    print("removing {0}".format(tmp_file))
                    os.remove(tmp_file)
