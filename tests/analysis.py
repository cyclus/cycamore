from __future__ import print_function
import test_regression as tst

import subprocess
from multiprocessing import Pool, Manager, cpu_count
from collections import defaultdict

diff_tbl = """table is different"""
diff_col = """Column"""

def add_tbl(dic, tbl):
    """adds a table in a defaultdict-like manner
    """
    if tbl in dic:
        dic[tbl] += 1
    else: 
        dic[tbl] = 1

def collect(args):
    """collects information on a given nosetest run
    """
    tbl_freq, col_freq = args
    rtn = subprocess.Popen(["nosetests", "-s"], stdout=subprocess.PIPE, 
                           stderr=subprocess.PIPE)
    out, err = rtn.communicate()
    for line in out.split("\n"):
        line = line.strip()
        if diff_tbl in line.strip():
            tbl_name = line.split()[0]
            add_tbl(tbl_freq, tbl_name)
        if diff_col in line.strip():
            col_name = line.split()[1]
            col_freq.append((tbl_name, col_name))

def proxy_lst_to_dict(lst):
    """converts the col_freq list into a dictionary for easier processing
    """
    col_freq = defaultdict(lambda: defaultdict(int))
    for tbl, col in lst:
        col_freq[tbl][col] += 1
    return col_freq

def determ_analysis(niter = 1000, fname = "report"):
    """
    Calls nosetests for a number of iterations and reports findings of
    nondeterminism to a file.

    Note that the tests that nosetest will call must be configured to test the
    supposed deterministic cases.
    """
    m = Manager()

    tbl_freq = m.dict()
    col_freq = m.list()

    # collect
    nproc = cpu_count()
    count = nproc if nproc == 1 else nproc - 1
    pool  = Pool(count)
    args = ((tbl_freq, col_freq) for i in range(niter))
    pool.map_async(collect, args)
    pool.close()
    pool.join()

    # convert from proxy
    col_freq = proxy_lst_to_dict(col_freq)
    tbl_freq = {item[0]: item[1] for item in tbl_freq.items()}

    # normalize
    for tbl, dic in col_freq.iteritems():
        for col, freq in dic.iteritems():
            dic[col] = "{0:.2f}".format(float(freq) / tbl_freq[tbl])    
    for k, v in tbl_freq.iteritems():
        tbl_freq[k] = "{0:.2f}".format(float(v) / niter)
    
    # report
    lines = []
    for tbl, freq in tbl_freq.iteritems():
        lines.append(tbl + " " + freq + "\n")
        for col, freq in col_freq[tbl].iteritems():
            lines.append("  " + col + " " + freq + "\n") 
    with open(fname, "w") as f:
        f.writelines(lines)

if __name__ == "__main__":
    niter = 100
    fname = "report"
    determ_analysis(niter, fname)
