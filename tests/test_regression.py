#! /usr/bin/env python

import os
import json
import hashlib
import urllib
import uuid
from nose.tools import assert_true
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
            urllib.urlretrieve(base_url+r["fname"], fpath)
        h = hashlib.sha1()
        with open(fpath, "rb") as f: 
            h.update(f.read())
        if h.hexdigest() != r["sha1-checksum"]:
            raise RuntimeError("They tooks our data!!! All our rackspace are belong to them.")
        sim_files[r["input-file"]] = fpath

def test_regression(check_deterministic=False):
    """Test for all inputs in sim_files. Checks if reference and current cyclus 
    output is the same.

    Parameters
    ----------
    check_deterministic : bool
                        If True, also test determinisitc equality of simulations

    WARNING: the tests require cyclus executable to be included in PATH
    """    
    for root, dirs, files in os.walk("../input"):
        for f in files:
            if f not in sim_files:
                continue
            
            tmp_file = str(uuid.uuid4()) + ".h5"
            run_cyclus("cyclus", os.getcwd(), os.path.join(root, f), tmp_file)
        
            if os.path.isfile(tmp_file):
                try:
                    nondeterm = compare_nondeterm(sim_files[f], tmp_file)
                except KeyError:
                    os.remove(tmp_file)
                    raise
                    
                if check_deterministic:
                    try:
                        determ = \
                            compare_determ(sim_files[f], tmp_file, verbose=True)
                    except KeyError:
                        os.remove(tmp_file)
                        raise

                os.remove(tmp_file)
                
                assert_true(nondeterm)
                if check_deterministic:
                    assert_true(determ) 
