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
            agent_entry = f.get_node("/AgentEntry")[:]
            agent_exit = f.get_node("/AgentExit")[:] if "/AgentExit" in f \
                else None
            resources = f.get_node("/Resources")[:]
            transactions = f.get_node("/Transactions")[:]
            
            # Find agent ids of source and sink facilities
            self.agent_ids = agent_entry["AgentId"]
            self.agent_impl = agent_entry["Spec"]
            self.depl_time = agent_entry["EnterTime"]
            if agent_exit:
                self.exit_ids = agent_exit["AgentId"]
                self.exit_time = agent_exit["ExitTime"]
            
            # Check transactions
            self.sender_ids = transactions["SenderId"]
            self.receiver_ids = transactions["ReceiverId"]
            self.trans_time = transactions["Time"]
            self.trans_resource = transactions["ResourceId"]
        
            # Track transacted resources
            self.resource_ids = resources["ResourceId"]
            self.quantities = resources["Quantity"]

    def teardown(self):
        if os.path.isfile(self.outf_):
            print("removing {0}".format(self.outf_))
            os.remove(self.outf_)

class TestPhysorEnrichment(TestRegression):
    def __init__(self):
        super(TestPhysorEnrichment, self).__init__()
        self.inf_ = "../input/physor/1_Enrichment_2_Reactor.xml"
        print(self.outf_)

    def setup(self):
        super(TestPhysorEnrichment, self).setup()
        self.rx_id = find_ids(":cycamore:BatchReactor", 
                              self.agent_impl, self.agent_ids)
        self.enr_id = find_ids(":cycamore:EnrichmentFacility", 
                               self.agent_impl, self.agent_ids)        

        with tables.open_file(self.outf_, mode="r") as f:
            self.enrichments = f.get_node("/Enrichments")[:]
        
    def test_deploy(self):
        # Test for 3 sources and 4 sinks are deployed in the simulation
        assert_equal(len(self.rx_id), 2)
        assert_equal(len(self.enr_id), 1)

    def test_swu(self):
        exp = [6.9, 10, 4.14, 6.9]
        obs = [np.sum(self.enrichments["SWU"][self.enrichments["Time"] == t]) \
                   for t in range(4)]
        np.testing.assert_almost_equal(exp, obs)
