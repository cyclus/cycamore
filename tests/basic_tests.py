
import subprocess

def test_cyclus_call():
    subprocess.call(['cyclus','-o','output.h5','--input-file',
    './null_sink.xml'])
    subprocess.call(['cyclus', '-o','output.h5','--input-file',
    './source_to_sink.xml'])
