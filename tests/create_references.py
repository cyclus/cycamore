#! /usr/bin/env python

import os

from cyclus_tools import run_cyclus
from testcases import sim_files

def main():
    """Creates reference databases. Assumes that cyclus is included into PATH.
    """
    cwd = os.getcwd()

    # Run cyclus
    run_cyclus("cyclus", cwd, sim_files)

if __name__ == "__main__":
    main()
