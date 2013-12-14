#! /usr/bin/python

import os
import sys
import shutil
from tools import check_cmd

def main(cyclus_version, cycamore_version):
    '''Creates reference databases for inpro cases
    in benchmarks folder.

    Arguments: version or sha of cyclus and cycamore to be checkout out by git
    '''
    dirs_to_clean = [] #keep track of directories to delete later
    try:
        cwd = os.getcwd()
        #Create Installation and Benchmarks Directories
        install_path = cwd + "/install"
        benchmarks_path = cwd + "/benchmarks"
        mkdir_safe(install_path)
        dirs_to_clean.append(install_path)
        mkdir_safe(benchmarks_path)

        #Create cyclus and cycamore repositories in the current directory
        get_repo("cyclus", cyclus_version, cwd)
        get_repo("cycamore", cycamore_version, cwd)
        cyclus_path = cwd + "/cyclus"
        cycamore_path = cwd + "/cycamore"
        dirs_to_clean.append(cyclus_path)
        dirs_to_clean.append(cycamore_path)

        #Install cyclus and cycamore
        install_project(cyclus_path, install_path)
        install_project(cycamore_path, install_path)
        cyclus = install_path + "/bin/cyclus" # cyclus executable

        #Run inpro cases
        run_inpros(cyclus, cwd)
    finally:
        #Clean up
        for dir in dirs_to_clean:
            shutil.rmtree(dir)

def mkdir_safe(path):
    ''' Create a non-existing directory '''
    if os.path.exists(path):
        if os.listdir(path) != []:
            raise OSError("Found a non-empty " + path)
    else:
        os.mkdir(path)

def get_repo(project, version, target_dir):
    ''' Gets repo from cyclus on gihub.'''
    holdsrtn = [1]
    repo_address = "https://github.com/cyclus/" + project + ".git"
    cmd_clone = ["git","clone", repo_address]
    check_cmd(cmd_clone, target_dir, holdsrtn)
    if holdsrtn[0] != 0 : sys.exit(1)
    cmd_version = ["git", "checkout", version]
    project_dir = target_dir + "/" + project
    check_cmd(cmd_version, project_dir, holdsrtn)
    if holdsrtn[0] != 0 : sys.exit(1)

def install_project(project_path, install_path):
    ''' Installs project by invoking install.py '''
    holdsrtn = [1]
    cmd_install = ["python", "install.py", "--prefix="+install_path]
    check_cmd(cmd_install, project_path, holdsrtn)
    if holdsrtn[0] != 0 : sys.exit(1)

def run_inpros(cyclus, cwd):
    '''Runs cyclus with various inpro inputs and creates reference databases
        in a benchmarks directory.'''
    #Cyclus simulation inputs
    sim_inputs = ["./inputs/inpro/inpro_low.xml",
                  "./inputs/inpro/inpro_high.xml",
                  "./inputs/inpro/inpro_low_short.xml",
                  "./inputs/inpro/inpro_high_short.xml",
                  "./inputs/inpro/inpro_low_no_decay.xml",
                  "./inputs/inpro/inpro_high_no_decay.xml"]
    #Benchmark databases must match the order of input files
    bench_dbs = ["./benchmarks/bench_inpro_low.h5",
                 "./benchmarks/bench_inpro_high.h5",
                 "./benchmarks/bench_inpro_low_short.h5",
                 "./benchmarks/bench_inpro_high_short.h5",
                 "./benchmarks/bench_inpro_low_no_decay.h5",
                 "./benchmarks/bench_inpro_high_no_decay.h5"]

    for sim_input,bench_db in zip(sim_inputs,bench_dbs):
        holdsrtn = [1] # needed because nose does not send() to test generator
        cmd = [cyclus, "-o", bench_db, "--input-file", sim_input]
        check_cmd(cmd, cwd, holdsrtn)
        rtn = holdsrtn[0]
        if rtn != 0:
            return # don't execute further commands

if __name__ == "__main__":
    if (len(sys.argv) == 1):
        # Default version to checkout
        cyclus_version = "0.3"
        cycamore_version = "0.3"
    elif (len(sys.argv) == 3):
        cyclus_version = sys.argv[1]
        cycamore_version = sys.argv[2]
    else:
        print("Needs exactly two arguments or none")
        sys.exit()

    main(cyclus_version, cycamore_version)
