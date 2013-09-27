
import subprocess

def test_cyclus_call():
    subprocess.call(['cyclus','-o','output.h5','--input-file',
    '$CYCAMORE_PATH/input/inpro/inpro_low.xml'])
    subprocess.call(['cyclus', '-o','output.h5','--input-file',
    '/home/olzhas/cyclus/cycamore/input/inpro/inpro_high.xml'])
