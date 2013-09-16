'''
Created on Sep 5, 2013

@author: olzhas
'''

from nose import *
from nose.tools import *

from subprocess import call
   
def test_ls():
    call(['ls','-alh'])
        
def test_print():
    call(['echo','On','Wisconsin'])
