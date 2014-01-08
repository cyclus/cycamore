#! /usr/bin/env python

from tools import check_cmd

def run_cyclus(cyclus, cwd, sim_files):
    """Runs cyclus with various inputs and creates output databases
       """

    for sim_input, sim_output in sim_files:
        holdsrtn = [1]  # needed because nose does not send() to test generator
        cmd = [cyclus, "-o", sim_output, "--input-file", sim_input]
        check_cmd(cmd, cwd, holdsrtn)
        rtn = holdsrtn[0]
        if rtn != 0:
            return  # don"t execute further commands
