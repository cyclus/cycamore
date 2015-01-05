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
            self.rsrc_qtys = {
                x["ResourceId"]: x["Quantity"] for x in self.resources}
            
            
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

        torxtrs = {i: [self.rsrc_qtys[x["ResourceId"]] \
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
        

class TestPhysorSources(TestRegression):
    def __init__(self):
        super(TestPhysorSources, self).__init__()
        self.inf_ = "../input/physor/2_Sources_3_Reactors.xml"

    def setup(self):
        super(TestPhysorSources, self).setup()
        
        # identify each reactor and supplier by id
        tbl = self.agent_entry
        rx_id = find_ids(":cycamore:BatchReactor", 
                         tbl["Spec"], tbl["AgentId"])
        self.r1, self.r2, self.r3 = tuple(rx_id)
        s_id = find_ids(":cycamore:Source", 
                        tbl["Spec"], tbl["AgentId"])
        self.smox = self.transactions[0]["SenderId"]
        s_id.remove(self.smox)
        self.suox = s_id[0]

    def test_rxtr_deployment(self):
        depl_time = {x["AgentId"]: x["EnterTime"] for x in self.agent_entry}
        
        assert_equal(depl_time[self.r1], 1)
        assert_equal(depl_time[self.r2], 2)
        assert_equal(depl_time[self.r3], 3)

    def test_rxtr1_xactions(self):
        xa = self.transactions
        
        mox_exp = [0, 1, 1, 1, 0]
        obs = np.zeros(5)
        rows = xa[np.logical_and(xa["ReceiverId"] == self.r1, 
                                 xa["SenderId"] == self.smox)] 
        obs[rows["Time"]] = [self.rsrc_qtys[x] for x in rows["ResourceId"]]
        np.testing.assert_almost_equal(mox_exp, obs)
        
        uox_exp = [0, 0, 0, 0, 1]
        obs = np.zeros(5)
        rows = xa[np.logical_and(xa["ReceiverId"] == self.r1, 
                                 xa["SenderId"] == self.suox)] 
        obs[rows["Time"]] = [self.rsrc_qtys[x] for x in rows["ResourceId"]]
        np.testing.assert_almost_equal(uox_exp, obs)
         
    def test_rxtr2_xactions(self):
        xa = self.transactions
        
        mox_exp = [0, 0, 1, 1, 1]
        obs = np.zeros(5)
        rows = xa[np.logical_and(xa["ReceiverId"] == self.r2, 
                                 xa["SenderId"] == self.smox)] 
        obs[rows["Time"]] = [self.rsrc_qtys[x] for x in rows["ResourceId"]]
        np.testing.assert_almost_equal(mox_exp, obs)
        
        uox_exp = [0, 0, 0, 0, 0]
        obs = np.zeros(5)
        rows = xa[np.logical_and(xa["ReceiverId"] == self.r2, 
                                 xa["SenderId"] == self.suox)] 
        obs[rows["Time"]] = [self.rsrc_qtys[x] for x in rows["ResourceId"]]
        np.testing.assert_almost_equal(uox_exp, obs)
         
    def test_rxtr3_xactions(self):
        xa = self.transactions
        
        mox_exp = [0, 0, 0, 0.5, 1]
        obs = np.zeros(5)
        rows = xa[np.logical_and(xa["ReceiverId"] == self.r3, 
                                 xa["SenderId"] == self.smox)] 
        obs[rows["Time"]] = [self.rsrc_qtys[x] for x in rows["ResourceId"]]
        np.testing.assert_almost_equal(mox_exp, obs)
        
        uox_exp = [0, 0, 0, 0.5, 0]
        obs = np.zeros(5)
        rows = xa[np.logical_and(xa["ReceiverId"] == self.r3, 
                                 xa["SenderId"] == self.suox)] 
        obs[rows["Time"]] = [self.rsrc_qtys[x] for x in rows["ResourceId"]]
        np.testing.assert_almost_equal(uox_exp, obs)
         
