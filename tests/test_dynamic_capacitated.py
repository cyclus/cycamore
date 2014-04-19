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

    A source facility is expected to offer a commodity of amount 1,
    and a sink faciliity is expected to request for a commodity of amount 1.
    Therefore, number of facilities correspond to the amounts of offers
    and requests.

    At time step 1, 3 source facilities and 2 sink facilities are deployed, and
    at time step 2, additional 2 sink facilities are deployed.
    According to this deployment schedule, at time step 1, only 2 transactions
    are expected, the number of sink facilities being the constraint; whereas,
    at time step 2, only 3 transactions are expected, the number of source
    facilities being the constraint.
    """
    # Cyclus simulation input for dynamic capacitated
    sim_inputs = ["./Inputs/dynamic_capacitated.xml"]

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
        depl_time = agent_entry["EnterTime"]

        source_id = find_ids("Source", agent_impl, agent_ids)
        sink_id = find_ids("Sink", agent_impl, agent_ids)

        # Test for 3 sources and 4 sinks are deployed in the simulation
        yield assert_equal, len(source_id), 3
        yield assert_equal, len(sink_id), 4

        # Test that source facilities are all deployed at time step 1
        for s in source_id:
            yield assert_equal, depl_time[np.where(agent_ids == s)], 1
        # Test that first 2 sink facilities are deployed at time step 1
        for i in [0, 1]:
            yield assert_equal,\
                    depl_time[np.where(agent_ids == sink_id[i])], 1
        for i in [2, 3]:
            yield assert_equal,\
                    depl_time[np.where(agent_ids == sink_id[i])], 2

        # Check transactions
        sender_ids = transactions["SenderId"]
        receiver_ids = transactions["ReceiverId"]
        trans_time = transactions["Time"]
        trans_resource = transactions["ResourceId"]
        # Track transacted resources
        resource_ids = resources["ResourceId"]
        quantities = resources["Quantity"]

        # Check that transactions are between sources and sinks only
        for s in sender_ids:
            yield assert_equal, len(np.where(source_id == s)), 1

        for r in receiver_ids:
            yield assert_equal, len(np.where(sink_id == s)), 1

        # Check that at time step 1, there are 2 transactions
        yield assert_equal, len(trans_time), 5
        yield assert_equal, len(np.where(trans_time == 1)[0]), 2
        # Check that at time step 2, there are 3 transactions
        yield assert_equal, len(np.where(trans_time == 2)[0]), 3

        # Check that at time step 1, there are 2 transactions with total
        # amount of 2
        quantity = 0
        for t in np.where(trans_time == 1)[0]:
            quantity += quantities[np.where(resource_ids == trans_resource[t])]
        yield assert_equal, quantity, 2

        # Check that at time step 2, there are 3 transactions with total
        # amount of 3
        quantity = 0
        for t in np.where(trans_time == 2)[0]:
            quantity += quantities[np.where(resource_ids == trans_resource[t])]
        yield assert_equal, quantity, 3

        output.close()
        os.remove("./output_temp.h5")
        # This is a starter sqlite db created implicitly
        os.remove("./output_temp.sqlite")
