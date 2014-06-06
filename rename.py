#! /usr/bin/env python
import os
import sys
import shutil
import subprocess
from glob import glob
import argparse as ap

absexpanduser = lambda x: os.path.abspath(os.path.expanduser(x))

def main():
    description = ("This script renames all instances of stub, Stub, and STUB "
                   "to the value given on the command line.")
    parser = ap.ArgumentParser(description=description)
    parser.add_argument('name', help="replacement for stub", default='name')
    ns = parser.parse_args()

    low, cap, upp = ns.name.lower(), ns.name.capitalize(), ns.name.upper()
    stublow, stubcap, stubupp = 'stub', 'Stub', 'STUB'
    mvfiles = glob('src/stub*')
    for f in mvfiles:
        d = os.path.dirname(f)
        name = os.path.basename(f)
        newname = os.path.join(d, name.replace('stub', low))
        subprocess.call(['git','mv',f,newname])
        subprocess.call(['git','commit','-m',
          '"moves stubfile '.join(name).join('->').join(newname).join('"')])

    files = ['CMakeLists.txt', 'input/example.xml'] + glob('src/*')
    for f in files:
        with open(f, 'r') as inp:
            s = inp.read()
        s = s.replace('stubs', low)
        s = s.replace(stublow, low)
        s = s.replace(stubcap, cap)
        s = s.replace(stubupp, upp)
        os.remove(f)
        d = os.path.dirname(f)
        name = os.path.basename(f)
        with open(os.path.join(d, name.replace('stub', low)), 'w') as out:
            out.write(s)

if __name__ == "__main__":
    main()
