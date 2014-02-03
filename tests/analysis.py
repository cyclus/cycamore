from __future__ import print_function
import test_regression as tst

import subprocess
from multiprocessing import Pool, Manager, cpu_count
from collections import defaultdict

diff_tbl = """table is different"""
diff_col = """Column"""

def collect(args):
    """collects information on a determinisitic regression test run
    """
    tbl_freq, col_freq = args

    rtn = subprocess.Popen(
        ["python", "-c", 
         "import test_regression as t; " +
         "t.setup(); t.test_regression(deterministic=True)"], 
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = rtn.communicate()
    #print(out, err)
    
    for line in out.split("\n"):
        line = line.strip()
        tbl_name = line.split()[0]
        col_name = line.split()[1]
        if diff_tbl in line.strip():
            tbl_freq[tbl_name] = \
                tbl_freq[tbl_name] + 1 if tbl_name in tbl_freq else 1
        if diff_col in line.strip():
            col_freq.append((tbl_name, col_name))

def proxy_lst_to_dict(lst):
    """converts the col_freq list into a dictionary for easier processing
    """
    col_freq = defaultdict(lambda: defaultdict(int))
    for tbl, col in lst:
        col_freq[tbl][col] += 1
    return col_freq

def determ_analysis(niter=1000, fname="report"):
    """
    Calls deterministic regression tests for a number of iterations and reports
    findings of nondeterminism to a file.

    Parameters
    ----------
    niter : int
          The number of times to run regression tests
         
    fname : str
          The output filename to report to
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
    """This module, by default will run 100 instances of the regression tests
    and send its findings to a file named 'report'.
    """
    determ_analysis(niter=100)
