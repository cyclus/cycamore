cwd=${PWD}

#Creating cyclus copy
git clone https://github.com/rakhimov/cyclus.git
cd $cwd/cyclus
git remote add upstream https://github.com/cyclus/cyclus.git
git fetch upstream
git branch master origin/master
git checkout master 

#Creating cycamore copy
cd $cwd
git clone https://github.com/rakhimov/cycamore.git
cd $cwd/cycamore
git remote add upstream https://github.com/cyclus/cycamore.git
git branch master origin/master
git checkout master

#Creating installation folder
cd $cwd 
mkdir install
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

