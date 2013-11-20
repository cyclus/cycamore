cwd=${PWD}

# Create installation directory
mkdir install
#Creating cyclus copy
git clone https://github.com/cyclus/cyclus.git
cd $cwd/cyclus
git checkout 0.3 

#Creating cycamore copy
cd $cwd
git clone https://github.com/cyclus/cycamore.git
cd $cwd/cycamore
git checkout 0.3 

#Installing cyclus
cd $cwd/cyclus
python install.py --prefix=../install

#Installing cycamore
cd $cwd/cycamore
python install.py --prefix=../install

#Running all the tests
cd $cwd/install/bin
python run_inputs.py
./Cyc*
./cyclus_*

#Returning to the original directory
cd $cwd
alias cyclus=$cwd/install/bin/cyclus

#Calling a python script to create references
python ./create_inpro_references.py

#Cleaning up
rm -rf ./cyclus
rm -rf ./install
rm -rf ./cycamore
unalias cyclus
cwd=''
