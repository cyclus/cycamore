
import subprocess
   
def test_ls():
    subprocess.call(['ls','-alh'], shell=True,stdout=False)
        
def test_print():
    subprocess.call(['echo','On','Wisconsin'],shell=True, stdout=False)
