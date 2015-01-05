#! /usr/bin/env python

from nose.tools import assert_equal, assert_true
import os
import tables
import numpy as np
import uuid
from tools import check_cmd
from helper import table_exist, find_ids
from cyclus_tools import run_cyclus

class TestRegression(object):
    def __init__(self):
        self.outf_ = str(uuid.uuid4()) + ".h5"
        self.inf_ = None

    def __del__(self):
        if os.path.isfile(self.outf_):
            print("removing {0}".format(self.outf_))
            os.remove(self.outf_)

    def setup(self):
        if not self.inf_:
            raise TypeError(("self.inf_ must be set in derived classes "
                             "to run regression tests."))
        run_cyclus("cyclus", os.getcwd(), self.inf_, self.outf_)        

        with tables.open_file(self.outf_, mode="r") as f:
            # Get specific tables and columns
            self.agent_entry = f.get_node("/AgentEntry")[:]
            self.agent_exit = f.get_node("/AgentExit")[:] if "/AgentExit" in f \
                else None
            self.resources = f.get_node("/Resources")[:]
            self.transactions = f.get_node("/Transactions")[:]
            
    def teardown(self):
        if os.path.isfile(self.outf_):
            print("removing {0}".format(self.outf_))
            os.remove(self.outf_)

class TestPhysorEnrichment(TestRegression):
    def __init__(self):
        super(TestPhysorEnrichment, self).__init__()
        self.inf_ = "../input/physor/1_Enrichment_2_Reactor.xml"

    def setup(self):
        super(TestPhysorEnrichment, self).setup()
        tbl = self.agent_entry
        self.rx_id = find_ids(":cycamore:BatchReactor", 
                              tbl["Spec"], tbl["AgentId"])
        self.enr_id = find_ids(":cycamore:EnrichmentFacility", 
                               tbl["Spec"], tbl["AgentId"])

        with tables.open_file(self.outf_, mode="r") as f:
            self.enrichments = f.get_node("/Enrichments")[:]
        
    def test_deploy(self):
        assert_equal(len(self.rx_id), 2)
        assert_equal(len(self.enr_id), 1)

    def test_swu(self):
        enr = self.enrichments
        exp = [6.9, 10, 4.14, 6.9]
        obs = [np.sum(enr["SWU"][enr["Time"] == t]) for t in range(4)]
        np.testing.assert_almost_equal(exp, obs, decimal=2)

    def test_nu(self):
        enr = self.enrichments
        exp = [13.03, 16.54, 7.83, 13.03]
        obs = [np.sum(enr["Natural_Uranium"][enr["Time"] == t]) \
                   for t in range(4)]
        np.testing.assert_almost_equal(exp, obs, decimal=2)

    def test_xactions(self):
        xa = self.transactions
        rs = self.resources

        rqtys = {x["ResourceId"]: x["Quantity"] for x in rs \
                     if x["ResourceId"] in xa["ResourceId"]}
        torxtrs = {i: [rqtys[x["ResourceId"]] \
                           for x in xa[xa["ReceiverId"] == i]] \
                           for i in self.rx_id} 
        transfers = sorted(torxtrs.values())

        exp = [1, 0.8, 0.2, 1]
        obs = transfers[0]
        msg = "Testing that first reactor gets less than it wants."      
        np.testing.assert_almost_equal(exp, obs, decimal=2, err_msg=msg)
        
        exp = [1, 1, 1, 1]
        obs = transfers[1]
        msg = "Testing that second reactor gets what it wants."      
        np.testing.assert_almost_equal(exp, obs, decimal=2)
        
