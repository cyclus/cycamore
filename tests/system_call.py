
import subprocess

def test_ls():
    subprocess.call(['ls','-alh'], shell=True,stdout=False)

def test_print():
    subprocess.call(['echo','On','Wisconsin'],shell=True, stdout=False)

def test_cyclus_unit_tests():
    subprocess.call(['/home/olzhas/cyclus/install/bin/CyclusUnitTestDriver'])
    subprocess.call(['/home/olzhas/cyclus/install/bin/CycamoreUnitTestDriver'])
    subprocess.call(['python','home/olzhas/cyclus/install/bin/run_inputs.py'])
def test_cyclus_call():
    subprocess.call(['cyclus', './null_sink.xml'])
    subprocess.call(['cyclus', './source_to_sink.xml'])
