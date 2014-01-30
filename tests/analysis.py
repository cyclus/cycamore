from __future__ import print_function
import test_regression as tst

import subprocess
from collections import defaultdict

def determ_analysis(niter = 1000, fname = "report"):
    """
    Calls nosetests for a number of iterations and reports findings of
    nondeterminism to a file.

    Note that the tests that nosetest will call must be configured to test the
    supposed deterministic cases.
    """
    tbl_freq = defaultdict(int)
    col_freq = defaultdict(lambda: defaultdict(int))
    
    # collect
    for i in range(niter):
        rtn = subprocess.Popen(["nosetests", "-s"], stdout=subprocess.PIPE, 
                               stderr=subprocess.PIPE)
        out, err = rtn.communicate()
        for line in out.split("\n"):
            line = line.strip()
            if "table is different" in line.strip():
                tbl_name = line.split()[0]
                tbl_freq[tbl_name] += 1 
            if "Column" in line.strip():
                col_freq[tbl_name][line.split()[1]] += 1
    # normalize
    for tbl, dic in col_freq.iteritems():
        for col, freq in dic.iteritems():
            dic[col] = "{0:.2f}".format(float(freq) / tbl_freq[tbl])
    for k, v in tbl_freq.iteritems():
        tbl_freq[k] = "{0:.2f}".format(float(v) / n)
    
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
    fname = "null_report"
    determ_analysis(niter, fname)
