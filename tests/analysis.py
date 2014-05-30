from __future__ import print_function
from __future__ import division

import subprocess
from multiprocessing import Pool, Manager, cpu_count
from collections import defaultdict
import argparse as ap
import time 

import test_regression as tst

diff_tbl = """table is different"""
diff_col = """Column"""

def collect(args):
    """collects information on a determinisitic regression test run
    """
    tbl_freq, col_freq = args

    rtn = subprocess.Popen(
        ["python", "-c", 
         "import test_regression as t; " +
         "t.setup(); obj = t.TestRegression();" +
         "obj.test_regression(check_deterministic=True)"], 
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = rtn.communicate()
    #print out, err

    for line in out.split("\n"):
        line = line.strip()
        if diff_tbl in line.strip():
            tbl_name = line.split()[0]
            tbl_freq[tbl_name] = \
                tbl_freq[tbl_name] + 1 if tbl_name in tbl_freq else 1
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

def determ_analysis(niter=1000):
    """
    Calls deterministic regression tests for a number of iterations and reports
    findings of nondeterminism to a file.

    Parameters
    ----------
    niter : int
          The number of times to run regression tests
         
    fname : str
          The output filename to report to

    Returns
    -------
    tbl_freq, col_freq : 2-tuple of dicts
                       tbl_freq is a frequency map of nondeterministic tables
                       col_freq is a frequency map of nondeterminisitc columns, 
                       per table
    """
    m = Manager()

    tbl_freq = m.dict()
    col_freq = m.list()

    # collect
    nproc = cpu_count()
    count = nproc if nproc == 1 else nproc - 1
    pool  = Pool(count)

    print("Beginning iterations on " + str(nproc) + " processors.")
    args = ((tbl_freq, col_freq) for i in range(niter))
    jobs = pool.map_async(collect, args)
    while not jobs.ready():
        print('{0:.1%} of jobs left to start.'.format(
                jobs._number_left / niter))
        time.sleep(5.0)
    if not jobs.successful():
        raise ValueError("At least one job failed.")
    pool.close()
    pool.join()
    print("Finished iterations.")

    # convert from proxy
    col_freq = proxy_lst_to_dict(col_freq)
    tbl_freq = {item[0]: item[1] for item in tbl_freq.items()}

    # normalize
    for tbl, dic in col_freq.iteritems():
        for col, freq in dic.iteritems():
            dic[col] = "{0:.2f}".format(float(freq) / tbl_freq[tbl])    
    for k, v in tbl_freq.iteritems():
        tbl_freq[k] = "{0:.2f}".format(float(v) / niter)

    return tbl_freq, col_freq

def report(tbl_freq, col_freq, fname="report"):
    """
    Prints the results of determ_analysis to a file

    Parameters
    ----------
    tbl_freq : dict
             the table frequency output from determ_analysis
         
    col_freq : dict
             the column frequency output from determ_analysis

    fname : str
          the output file name to print to
    """
    lines = []
    lines.append("Table values are reported as percent nondeterministic" + 
                 " of total runs.\n\n")
    lines.append("Column values are reported as percent nondeterministic" +
                 " of all table nondeterminism occurrences.\n\n")
    if len(tbl_freq) == 0:
        lines.append("No nondeterminism found.")
    for tbl, freq in tbl_freq.iteritems():
        lines.append(tbl + " " + freq + "\n")
        for col, freq in col_freq[tbl].iteritems():
            lines.append("  " + col + " " + freq + "\n") 
    with open(fname, "w") as f:
        f.writelines(lines)


def main():
    description = "A module for analyzing the (non)determinism of Cyclus output."

    parser = ap.ArgumentParser(description=description)

    niter = 'the number of regression test runs to perform'
    parser.add_argument('-n', '--niterations', type=int, help=niter, 
                        default=100)

    out = 'the file to write the report to'
    parser.add_argument('--out', help=out, default='report')
    
    args = parser.parse_args()
    tbl_freq, col_freq = determ_analysis(args.niterations)
    report(tbl_freq, col_freq, args.out)

if __name__ == "__main__":
    main()
