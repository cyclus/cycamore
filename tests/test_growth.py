#! /usr/bin/env python

from nose.tools import assert_equal, assert_true
import os
import tables
import numpy as np
import uuid
from helper import check_cmd, table_exist, find_ids

def test_growth():
    """Tests GrowthRegion, ManagerInst, and Source over a 4-time step
    simulation.

    A linear growth demand (y = x + 2) is provided to the growth region. Two
    Sources are allowed in the ManagerInst, with capacities of 2 and 1.1,
    respectively. At t=1, a 2-capacity Source is expected to be built, and at
    t=2 and t=3, 1-capacity Sources are expected to be built.
    """
    holdsrtn = [1]  # needed because nose does not send() to test generator
    sim_input = "./input/growth.xml"
    tmp_file = str(uuid.uuid4()) + ".h5"
    cmd = ["cyclus", "-o", tmp_file, "--input-file", sim_input]
    yield check_cmd, cmd, '.', holdsrtn
    rtn = holdsrtn[0]
    if rtn != 0:
        return  # don't execute further commands
    
    output = tables.open_file(tmp_file, mode = "r")
    # Tables of interest
    paths = ["/AgentEntry",]
    # Check if these tables exist
    yield assert_true, table_exist(output, paths)
    if not table_exist(output, paths):
        output.close()
        if os.path.isfile(tmp_file):
            print("removing {0}".format(tmp_file))
            os.remove(tmp_file)
            return  # don't execute further commands

    # Get specific tables and columns
    agent_entry = output.get_node("/AgentEntry")[:]

    # Find agent ids of source and sink facilities
    agent_ids = agent_entry["AgentId"]
    proto = agent_entry["Prototype"]
    depl_time = agent_entry["EnterTime"]

    source1_id = find_ids("Source1", proto, agent_ids)
    source2_id = find_ids("Source2", proto, agent_ids)
    
    assert_equal(len(source2_id), 1)
    assert_equal(len(source1_id), 2)

    assert_equal(depl_time[np.where(agent_ids == source2_id[0])], 1)
    assert_equal(depl_time[np.where(agent_ids == source1_id[0])], 2)
    assert_equal(depl_time[np.where(agent_ids == source1_id[1])], 3)
        
    output.close()
    if os.path.isfile(tmp_file):
        print("removing {0}".format(tmp_file))
        os.remove(tmp_file)
