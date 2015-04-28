"""# This is a simple tool to update the cyclus_nuc_data.h5 taken from pyne's upload.py

Note that this script is run automatically in maintenence.sh! You don't have to
run it separately if you're using that tool.

In order to use this script you need to create an rs.cred file from the api
information on the google drive doc.

Prerequisites:

 * PyNE
 * pyrax

Update procedure:

 * Update and install Pyne
 * run the following command in this folder:: 

 nuc_data_make -o cyclus_nuc_data.h5 \
 -m atomic_mass,scattering_lengths,decay,simple_xs,materials,eaf,wimsd_fpy,nds_fpy

 * then run::
 
 python upload.py
 
 * The cyclus prebuilt_nuc_data.h5 should now be on rackspace it may take 12-24
   hours for this to propagate to all the CDN nodes.

"""
from __future__ import print_function
import pyrax
import os

def push_rackspace(fname, cred_file='rs.cred'):
    pyrax.set_credential_file(cred_file)
    cf = pyrax.cloudfiles
    with open(fname, 'rb') as f:
        fdata = f.read()
    cont = cf.get_container("cyclus-data")
    obj = cf.store_object("cyclus-data", fname, fdata)
    cont.purge_cdn_object(fname)

def setup():
    pyrax.set_setting("identity_type", "rackspace")
    pyrax.set_setting('region', 'ORD')
    pyrax.set_credential_file('rs.cred')
    
def main():
    setup()
    cf = pyrax.cloudfiles
    print("list_containers: {}".format(cf.list_containers()))
    print("get_all_containers: {}".format(cf.get_all_containers()))
    push_rackspace('cyclus_nuc_data.h5')

if __name__ == '__main__':
    main()
