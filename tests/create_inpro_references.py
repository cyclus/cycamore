
#! /usr/bin/python

import sys
import subprocess
from tools import check_cmd

def run_inpros():
    #Cyclus simulation inputs
    sim_inputs = ["./inputs/inpro_low.xml",
                  "./inputs/inpro_high.xml", 
                  "./inputs/inpro_low_short.xml",
                  "./inputs/inpro_low_no_decay.xml",
                  "./inputs/inpro_high_no_decay.xml"]
    #Benchmark databases must match the order of input files
    bench_dbs = ["./benchmarks/bench_inpro_low.h5",
                 "./benchmarks/bench_inpro_high.h5",
                 "./benchmarks/bench_inpro_low_short.h5",
                 "./benchmarks/bench_inpro_low_no_decay.h5",
                 "./benchmarks/bench_inpro_high_no_decay.h5"]

    for sim_input,bench_db in zip(sim_inputs,bench_dbs):
        holdsrtn = [1] # needed because nose does not send() to test generator
        cmd = ["cyclus", "-o", bench_db, "--input-file", sim_input]
        check_cmd(cmd, '.', holdsrtn)
        rtn = holdsrtn[0]
        if rtn != 0:
            return # don't execute further commands

if __name__ == "__main__":
    
    run_inpros()
