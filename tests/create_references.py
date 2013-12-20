#! /usr/bin/env python

import os
import sys
import shutil
import argparse as ap

from tools import check_cmd

def main():
    """Creates reference databases for physor cases
    in benchmarks folder.

    Arguments: version or sha of cyclus and cycamore to be checkout out by git
               a name of scenario
    """
    description = "A Cyclus reference database creator."
    parser = ap.ArgumentParser(description=description)

    cyclus_version = "Version or sha of git that is used to build Cyclus."
    parser.add_argument("--cyclus_version", help=cyclus_version, default="0.3")

    cycamore_version = "Version or sha of git that is used to build Cycamore."
    parser.add_argument("--cycamore_version", help=cycamore_version, default="0.3")

    scenario = "Predefined scenario package name."
    parser.add_argument("--scenario", help=scenario)

    repo_owner = "Owner of cyclus and cycamore repos on github.com"
    parser.add_argument("--repo_owner", help=repo_owner, default="cyclus")

    input_source = "Where input files should be taken from. The default is local."
    parser.add_argument("--input_source", help=input_source, default="local") 

    args = parser.parse_args()

    cwd = os.getcwd()

    if (args.scenario == "physor"):
        # Cyclus simulation inputs for physor
        sim_inputs = ["./inputs/physor/1_Enrichment_2_Reactor.xml",
                      "./inputs/physor/2_Sources_3_Reactors.xml"]
        # Benchmark databases for physor must match the order of input files
        bench_dbs = [cwd + "/benchmarks/physor_1_Enrichment_2_Reactor.h5",
                     cwd + "/benchmarks/physor_2_Sources_3_Reactors.h5"]
    elif (args.scenario == "inpro"):
        # Cyclus simulation inputs
        sim_inputs = ["./inputs/inpro/inpro_low.xml",
                      "./inputs/inpro/inpro_high.xml",
                      "./inputs/inpro/inpro_low_short.xml",
                      "./inputs/inpro/inpro_high_short.xml",
                      "./inputs/inpro/inpro_low_no_decay.xml",
                      "./inputs/inpro/inpro_high_no_decay.xml"]
        # Benchmark databases must match the order of input files
        bench_dbs = [cwd + "/benchmarks/bench_inpro_low.h5",
                     cwd + "/benchmarks/bench_inpro_high.h5",
                     cwd + "/benchmarks/bench_inpro_low_short.h5",
                     cwd + "/benchmarks/bench_inpro_high_short.h5",
                     cwd + "/benchmarks/bench_inpro_low_no_decay.h5",
                     cwd + "/benchmarks/bench_inpro_high_no_decay.h5"]
    else:
        print("No legitimate scenario is provided")
        sys.exit()


    dirs_to_clean = []  # keep track of directories to delete later
    try:
        main_body(args.cyclus_version,
                  args.cycamore_version,
                  args.repo_owner,
                  dirs_to_clean,
                  args.input_source,
                  sim_inputs,
                  bench_dbs)
    finally:
        # Clean up
        for dir in dirs_to_clean:
            shutil.rmtree(dir)

def main_body(cyclus_version,
               cycamore_version,
               repo_owner,
               dirs_to_clean,
               input_source,
               sim_inputs,
               bench_dbs):
    """The body of main that creates reference databases"""
    cwd = os.getcwd()
    # create installation and benchmarks directories
    install_path = cwd + "/install"
    benchmarks_path = cwd + "/benchmarks"
    mkdir_safe(install_path)
    dirs_to_clean.append(install_path)
    mkdir_safe(benchmarks_path)

    # create cyclus and cycamore repositories in the current directory
    get_repo("cyclus", repo_owner, cyclus_version, cwd)
    get_repo("cycamore", repo_owner, cycamore_version, cwd)
    cyclus_path = cwd + "/cyclus"
    cycamore_path = cwd + "/cycamore"
    dirs_to_clean.append(cyclus_path)
    dirs_to_clean.append(cycamore_path)

    # install cyclus and cycamore
    install_project(cyclus_path, install_path)
    install_project(cycamore_path, install_path)
    cyclus = install_path + "/bin/cyclus"  # cyclus executable

    if (input_source == "remote"):
        inputs_location = cycamore_path + "/tests/"
    elif (input_source == "local"):
        inputs_location = cwd
    else:
        print("input_source is not identified correctly")
        sys.exit()

    # Run cyclus
    run_cyclus(cyclus, inputs_location, sim_inputs, bench_dbs)

def mkdir_safe(path):
    """Create a non-existing directory"""
    if os.path.exists(path):
        if os.listdir(path) != []:
            print("WARNING: A non-empty " + path + "\nABORT MANUALLY TO AVOID OVERWRITING")
    else:
        os.mkdir(path)

def get_repo(project, repo_owner, version, target_dir):
    """Gets repo from cyclus on gihub."""
    holdsrtn = [1]
    repo_address = "https://github.com/" + repo_owner +"/"+ project + ".git"
    cmd_clone = ["git", "clone", repo_address]
    check_cmd(cmd_clone, target_dir, holdsrtn)
    if holdsrtn[0] != 0 : sys.exit(1)
    cmd_version = ["git", "checkout", version]
    project_dir = target_dir + "/" + project
    check_cmd(cmd_version, project_dir, holdsrtn)
    if holdsrtn[0] != 0 : sys.exit(1)

def install_project(project_path, install_path):
    """Installs project by invoking install.py"""
    holdsrtn = [1]
    cmd_install = ["python", "install.py", "--prefix=" + install_path]
    check_cmd(cmd_install, project_path, holdsrtn)
    if holdsrtn[0] != 0 : sys.exit(1)

def run_cyclus(cyclus, cwd, sim_inputs, bench_dbs):
    """Runs cyclus with various inputs and creates reference databases
        in a benchmarks directory."""

    for sim_input, bench_db in zip(sim_inputs, bench_dbs):
        holdsrtn = [1]  # needed because nose does not send() to test generator
        cmd = [cyclus, "-o", bench_db, "--input-file", sim_input]
        check_cmd(cmd, cwd, holdsrtn)
        rtn = holdsrtn[0]
        if rtn != 0:
            return  # don"t execute further commands

if __name__ == "__main__": main()
