
import subprocess

def test_cyclus_call():
    subprocess.call(['cyclus', './null_sink.xml'])
    subprocess.call(['cyclus', './source_to_sink.xml'])
