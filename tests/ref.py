#!/usr/bin/python

from __future__ import print_function
import hashlib
import subprocess as sp
import argparse
import json
import os.path
import uuid
import multiprocessing
import shutil
import importlib
import sys
import pyrax
pyrax.set_setting("identity_type", "rackspace")

def gen_main(args):
    # create sandbox dir
    sandbox_path = '/tmp/refgen-' + str(uuid.uuid4())
    install_path = os.path.join(sandbox_path, 'install')
    os.makedirs(install_path)

    # fetch and build cyclus
    cyclus_path = os.path.join(sandbox_path, 'cyclus')
    build_path = prepare_repo(args.cyclus_repo, cyclus_path)
    tags = repo_tags(cyclus_path)
    if args.cyclus_refspec == '':
        args.cyclus_refspec = tags[-1]
    checkout(cyclus_path, args.cyclus_refspec)
    build_cyclus(build_path, install_path, prefix_path = args.cmake_prefix, boost = args.boost_root, coin = args.coin_root)

    # fetch and build cycamore
    cycamore_path = os.path.join(sandbox_path, 'cycamore')
    build_path = prepare_repo(args.cycamore_repo, cycamore_path)
    tags = repo_tags(cycamore_path)
    if args.cycamore_refspec == '':
        args.cycamore_refspec = tags[-1]
    checkout(cycamore_path, args.cycamore_refspec)
    build_cycamore(build_path, install_path, prefix_path = args.cmake_prefix, boost = args.boost_root, coin = args.coin_root)

    # run cyclus simulations
    sys.path.insert(0, os.path.join(cycamore_path, "tests"))
    mod = importlib.import_module("test_cases")
    for infile in mod.sim_files:
        run_cyclus(install_path, infile, args.cyclus_refspec, args.cycamore_refspec)

    # cleanup
    shutil.rmtree(sandbox_path)

def prepare_repo(url, dst_path):
    """ Clones a repo, creates a build dir.
    """
    sp.check_call(['git', 'clone', url, dst_path])
    build_path = os.path.join(dst_path, "build")
    os.makedirs(build_path)
    return build_path

def checkout(repo_path, commit):
    """ Checks out the specified commit in a repo.
    """
    cwd = os.getcwd()
    os.chdir(repo_path)
    sp.check_call(['git', 'checkout', commit])
    os.chdir(cwd)

def repo_tags(repo_path):
    """ Returns a sorted list of all tags in the repo (most recent last).
    """
    #git tag | xargs -I@ git log --format=format:"%ai @%n" -1 @ | sort | awk '{print $4}'
    cwd = os.getcwd()
    os.chdir(repo_path)
    git = sp.Popen(['git', 'tag'], stdout = sp.PIPE)
    xargs = sp.Popen(['xargs', '-I@', 'git', 'log', '--format=format:"%ai @%n"', '-1', '@'], stdin = git.stdout, stdout = sp.PIPE)
    sort = sp.Popen(['sort'], stdin = xargs.stdout, stdout = sp.PIPE)
    awk = sp.Popen(['awk', '{print $4}'], stdin = sort.stdout, stdout = sp.PIPE)
    (tagstext, _) = awk.communicate()
    tags = tagstext.decode().split('\n')[1:-1]
    os.chdir(cwd)
    return tags

def build_cyclus(build_path, install_path, prefix_path = '', boost = '', coin = ''):
    cmd = ['cmake', '..', '-DCMAKE_INSTALL_PREFIX=' + os.path.abspath(install_path)]
    if prefix_path != '':
        cmd.append('-DCMAKE_PREFIX_PATH=' + os.path.abspath(prefix_path))
    if boost != '':
        cmd.append('-DBOOST_ROOT=' + os.path.abspath(boost))
    if coin != '':
        cmd.append('-DCOIN_ROOT_DIR=' + os.path.abspath(coin))
    cwd = os.getcwd()
    os.chdir(build_path)

    sp.check_call(cmd)
    sp.check_call(['make', '-j', str(multiprocessing.cpu_count())])
    sp.check_call(['make', 'install'])
    os.chdir(cwd)

def build_cycamore(build_path, install_path, prefix_path = '', boost = '', coin = ''):
    cmd = ['cmake', '..', '-DCMAKE_INSTALL_PREFIX=' + os.path.abspath(install_path), '-DCYCLUS_ROOT_DIR=' + install_path]
    if prefix_path != '':
        cmd.append('-DCMAKE_PREFIX_PATH=' + os.path.abspath(prefix_path))
    if boost != '':
        cmd.append('-DBOOST_ROOT=' + os.path.abspath(boost))
    if coin != '':
        cmd.append('-DCOIN_ROOT_DIR=' + os.path.abspath(coin))
    cwd = os.getcwd()
    os.chdir(build_path)

    sp.check_call(cmd)
    sp.check_call(['make', '-j', str(multiprocessing.cpu_count())])
    sp.check_call(['make', 'install'])
    os.chdir(cwd)

def run_cyclus(install_path, input_file, cyclus_ref, cycamore_ref):
    out_file = encode_dbname(cyclus_ref, cycamore_ref, input_file)
    cyclus = os.path.join(install_path, 'bin', 'cyclus')
    sp.check_call([cyclus, '-o', out_file, input_file])

def encode_dbname(cyclus_ref, cycamore_ref, input_file):
    in_name, _ = os.path.splitext(os.path.basename(input_file))
    return cyclus_ref + '_' + cycamore_ref + '_' + in_name + '.h5'

def decode_dbname(fname):
    base, _ = os.path.splitext(fname)
    parts = base.split('_')
    cyclus_ref = parts[0]
    cycamore_ref = parts[1]
    infile = '_'.join(parts[2:]) + '.xml'
    return (cyclus_ref, cycamore_ref, infile)

def add_main(args):
    # retrieve existing reflist
    reflist = []
    if os.path.exists(args.reflist):
        with open(args.reflist) as f:
            data = f.read()
            reflist = json.loads(data)

    # add new ref dbs and hashes to reflist
    for refname in args.ref_dbs:
        if refname in [entry['fname'] for entry in reflist]:
            print(refname + ' is already in reflist \'' + args.reflist + '\'')

        with open(refname, 'rb') as f:
            data = f.read()
            h = hashlib.sha1()
            h.update(data)
        (cyclus_ref, cycamore_ref, infile) = decode_dbname(refname)
        reflist.append({
            'fname': refname,
            'sha1-checksum': h.hexdigest(),
            'cyclus-ref': cyclus_ref,
            'cycamore-ref': cycamore_ref,
            'input-file': infile
            })

        push_rackspace(refname, args.rs_cred)

    # update reflist
    data = json.dumps(reflist, indent=4)
    with open(args.reflist, 'w') as f:
        f.write(data)
    
def push_rackspace(fname, cred_file='rs.cred'):
  creds_file = os.path.expanduser(cred_file)
  pyrax.set_credential_file(cred_file)
  cf = pyrax.cloudfiles
  with open(fname, 'rb') as f:
    fdata = f.read()
  obj = cf.store_object("cyclus", fname, fdata)

def fetch_refdbs(cyclus_ref, cycamore_ref, dst_path = '.'):
    cwd = os.getcwd()
    os.chdir(dst_path)
    os.chdir(cwd)

if __name__ == '__main__':
    desc = 'Generates cyclus reference databases for regression testing'
    p = argparse.ArgumentParser(description=desc)
    subs = p.add_subparsers(help='sub-command help')

    # gen subcommand
    sub_gen = subs.add_parser('gen', help='create a new reference db set')
    help = 'force generate new ref-db rather than fetch remotely'
    sub_gen.add_argument('-f,--force', type=bool, help=help, default=False)
    help = 'location of cycamore repo'
    sub_gen.add_argument('--cyclus-repo', help=help, default='https://github.com/cyclus/cyclus.git')
    help = 'location of cycamore repo'
    sub_gen.add_argument('--cycamore-repo', help=help, default='https://github.com/cyclus/cycamore.git')
    help = 'git refspec for cyclus commit to generate reference from. Blank defaults to the most recent tag.'
    sub_gen.add_argument('--cyclus-refspec', help=help, default='')
    help = 'git refspec for cycamore commit to generate reference from. Blank defaults to the most recent tag.'
    sub_gen.add_argument('--cycamore-refspec', help=help, default='')
    help = 'the relative path to the Coin-OR libraries directory'
    sub_gen.add_argument('--coin-root', help=help, default = '')
    help = 'the relative path to the Boost libraries directory'
    sub_gen.add_argument('--boost-root', help=help, default = '')
    help = 'the cmake prefix path for use with FIND_PACKAGE, '
    help += 'FIND_PATH, FIND_PROGRAM, or FIND_LIBRARY macros'
    sub_gen.add_argument('--cmake-prefix', help=help, default = '')
    sub_gen.set_defaults(func=gen_main)

    # add subcommand
    sub_add = subs.add_parser('add', help='add a new reference db set to the project')
    sub_add.add_argument('ref_dbs', metavar='FILE', nargs='+', help='list of reference db files')
    sub_add.add_argument('--reflist', help='filename of reflist', default='./reflist.json')
    sub_add.add_argument('--rs-cred', help='rackspace credentials file', default='rs.cred')
    sub_add.set_defaults(func=add_main)

    args = p.parse_args()
    args.func(args)

