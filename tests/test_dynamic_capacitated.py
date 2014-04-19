#! /usr/bin/python

from nose.tools import assert_equal, assert_true
from numpy.testing import assert_array_equal
import os
import tables
import numpy as np
from tools import check_cmd
from helper import table_exist, find_ids, exit_times

"""Tests"""
def test_dynamic_capacitated():
    """Tests dynamic capacity restrains involving changes in the number of
    source and sink facilities.
    """
    # Cyclus simulation input for dynamic capacitated
    sim_inputs = ["../input/dynamic_capacitated/dynamic_capacitated.xml"]

    for sim_input in sim_inputs:
        holdsrtn = [1]  # needed because nose does not send() to test generator
        cmd = ["cyclus", "-o", "./output_temp.h5", "--input-file", sim_input]
        yield check_cmd, cmd, '.', holdsrtn
        rtn = holdsrtn[0]
        if rtn != 0:
            os.remove("./output_temp.h5")
            # This is a starter sqlite db created implicitly
            os.remove("./output_temp.sqlite")
            return  # don't execute further commands

        output = tables.open_file("./output_temp.h5", mode = "r")
        # Tables of interest
        paths = ["/AgentEntry", "/Resources", "/Transactions", "/Info"]
        # Check if these tables exist
        yield assert_true, table_exist(output, paths)
        if not table_exist(output, paths):
            output.close()
            os.remove("./output_temp.h5")
            # This is a starter sqlite db created implicitly
            os.remove("./output_temp.sqlite")
            return  # don't execute further commands

        # Get specific tables and columns
        agent_entry = output.get_node("/AgentEntry")[:]
        info = output.get_node("/Info")[:]
        resources = output.get_node("/Resources")[:]
        transactions = output.get_node("/Transactions")[:]

        # Find agent ids of source and sink facilities
        agent_ids = agent_entry["AgentId"]
        agent_impl = agent_entry["Implementation"]

        source_id = find_ids("Source", agent_impl, agent_ids)
        sink_id = find_ids("Sink", agent_impl, agent_ids)

        # Test for 3 sources and 4 sinks are deployed in the simulation
        yield assert_equal, len(source_id), 3
        yield assert_equal, len(sink_id), 4

        # Check if transactions are only between source and sink
        sender_ids = transactions["SenderId"]
        receiver_ids = transactions["ReceiverId"]
        trans_time = transactions["Time"]
        # Track transacted resources
        resource_ids = resources["ResourceId"]
        quantities = resources["Quantity"]

        output.close()
        os.remove("./output_temp.h5")
        # This is a starter sqlite db created implicitly
        os.remove("./output_temp.sqlite")
