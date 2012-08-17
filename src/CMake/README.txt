_______________________________________________________________________
Cycstub : Cyclus Stub Models 
_______________________________________________________________________

**Last Updated: 8.8.2012**

This repository provides templates for creating modules for use with the 
Cyclus nuclear fuel cycle simulator from the University of Wisconsin - 
Madison. This repository is intended to support innovative fuel cycle 
simulations with the Cyclus fuel cycle simulator. 

To see user and developer documentation for the cyclus code, please visit the `Cyclus Homepage`_.


-----------------------------------------------------------------------
LISCENSE
-----------------------------------------------------------------------

::

    Copyright (c) 2010-2012, University of Wisconsin Computational Nuclear Engineering Research Group
     All rights reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    
      - Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      
      - Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      
      - Neither the name of the University of Wisconsin Computational
        Nuclear Engineering Research Group nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

------------------------------------------------------------------
The Purpose of Cycstub
------------------------------------------------------------------

One important goal of the Cyclus effort is to attract a community of developers
contributing to a vibrant ecosystem of models for use by users. In addition to the 
wide availability of the core infrastructure, an element that is critical to the 
success of this community is a low-barrier to adoption of the *Cyclus* framework. 
This Cycstub repository provides a template for quick-start development of fuel 
cycle models within the cyclus framework. 

Run-time modules, or plug-ins, developed with the use of the Cycstub templates can be 
developed and distributed under any possible licensing scheme. By providing this 
stub repository separately from the core infrastructure, the modules developed using
these stubs will be distributed separately from the core infrastructure.  The 
distribution responsibility will rest with the developer of each module. This 
system will insulate the core infrastructure from accidental “pollution” by modules
of a sensitive nature, and similarly limit issues regarding the authorization for
distribution to the author’s organization. Ideally, most module developers will be
authorized for open distribution of their modules, but if not, we recommend maintaining
a private git repository on authorized servers. 

Finally, the community will be relied upon to provide review and curation of available 
modules, establishing both quality assurance practices and recommendations for best use
cases for each contributed module.


------------------------------------------------------------------
How To Use Cycstub
------------------------------------------------------------------

Let's say you've decided to implement a new Facility model. Let's say it's a 
one group burnup approximation of some kind, intended to generically represent 
material transmutation in a reactor. You want to call it OneGroupReactor. A
quick way to integrate your reactor model into a *Cyclus* simulation will be to 
fork Cycstub, reconfigure it for your own use, and customize the StubFacility 
template within it. During this process you can either choose to keep your code to
yourself by keeping the code on private, authorized machines, distribute it freely 
on the github fork you've created, or something in between. Please keep in mind 
that module developers are solely responsible for distribution decisions of their 
modules.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Forking this Repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to download and use this repository in a way that will keep 
track of changes in the Cyclus model interface, you'll need to *Fork* this 
repository.  

First, if you're not already logged in, please 
`log into github <https://github.com/login/>`_ .

Once you're logged in and have navigated back to `this repository's page 
<https://github.com/cyclus/cycstub/>`, there will be a button in the upper right 
hand corner that says Fork. 

You've now forked this repository, but it now exists only on github, and it has 
the wrong name.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Renaming Your Fork
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Your fork is, by default, called cycstub. You may rather call it 
OneGroupReactor. To do this, go to your new fork (github should have taken you 
there automatically, but you can find it at https://github.com/username/cycstub 
).

In the upper right hand corner of the browser will be a button called admin. 
Click on that button and rename your fork.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Cloning and Configuring Your Fork
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

First, make sure you've `configured git on your computer 
<https://help.github.com/articles/set-up-git/>`.

Once that's done, you can clone your repository. 
In a terminal on your machine, run the following code, replacing username with 
your github user name and onegroupreactor with the new name of your fork.

::

  git clone https://github.com/username/onegroupreactor.git
 

Add a remote, read-only branch that points to the cycstub repo : 

::

  cd onegroupreactor
  git remote add cyclus git://github.com/cyclus/cycstub.git



~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Modifying the Stubs 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Remove Unneeded Stub Files
===========================

Since, in this example, you're only building a Facility model, you should 
delete all of the other model type directories. Note that not everyone 
will be building Facility models.  These instructions will apply analgously 
in your case, *mutatis mutandis*. 

::

  cd src/Models
  git rm -rf Market/ Inst/ Region/ Converter/
  git commit -am "removes unneccessary files"
  git push origin master


Search and Replace Stub 
=============================

Inside the files, there are references to StubFacility. 
Your model isn't called StubFacility. It's called OneGroupReactor. 
You should search for instances of StubFacility (and STUBFACILITY and stubfacility) 
and replace them with analogous OneGroupReactor text. 

( This task will soon be scripted.) 

Commit your changes and push them to your fork.

Rename Stub Files
=============================

Just as you removed references to Stub inside the files, you should now change all 
directories and files named StubFacility to files analogously named 
OneGroupReactor. ( This task will also soon be scripted. )

::

  cd src/Models/Facility
  git mv StubFacility OneGroupReactor
  cd OneGroupReactor
  git mv StubFacility.h OneGroupReactor.h
  git mv StubFacility.cpp OneGroupReactor.cpp
  git mv StubFacilityTests.h OneGroupReactorTests.h
  ....


Commit your changes and push them to your fork.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Implementing Your Model
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The model you've chosen to create can now be implemented within the 
OneGroupReactor.cpp and .h files as well as any other files you generate and 
include in the build system. 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Keeping your Model up-to-date
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Cyclus core doesn't promise to be backwards compatible, moving forward, 
though we'll make every attempt to keep changes to our interface very minimal.
If there are changes to the model interface, we'll make appropriate changes 
here, to the cycstub repository. To incorporate those changes into your model 
repository, you'll need to take just a few steps. 

Create a new branch. 

::

  git checkout -b update

Fetch the changes from our repository.

::

  git fetch cyclus/cycstub

Then, merge

::

  git merge cyclus/cycstub


Then, correct any instances of cycstub or StubFacility, or any such language, 
with your own model names. Check this readme for changelogs addressing the 
changes made for the interface.

Then merge the update branch into your main repository. 

:: 

  git checkout develop
  git merge update


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Letting Us Know
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We like to know when someone is developing a tool for Cyclus. Please send us an 
email when you get started. We can add you to the cyclus developers list, which 
may help you in the development process.


------------------------------------------------------------------
Building and Installing Your Module
------------------------------------------------------------------

The `Cyclus Homepage`_ has much more detailed guides and information.
This Readme is intended to be a quick reference for building and installing the 
the stub module libraries for the first time.

~~~~~~~~~~~~~~~~~~~~~~~~~~~
Dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Building a module for the Cyclus  code requires the following software and libraries.

====================   ==================
Package                Minimum Version   
====================   ==================
`Cyclus`               0.1  
`CMake`                2.8            
`boost`                1.34.1
`libxml2`              2                 
`sqlite3`              3.7.10            
====================   ==================

~~~~~~~~~~~~~~~~~~~~~~~~~~~
Building Your Module
~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to facilitate future compatibility with multiple platforms, Cyclus and 
its modules are, by default, built using  `Cmake <http://www.cmake.org>`_. This 
relies on CMake version 2.8 or higher and the CMakeLists.txt file in `src/`. It is
recommended that you use CMake to build your module libraries in a directory 
external to the source code. Now, to use your module(s) within a cyclus 
simulation, you must already have cyclus installed . Once that is done, build your
module libraries by the following steps::

    .../onegroupreactor/$ mkdir build
    .../onegroupreactor/$ cd build
    .../onegroupreactor/build$ cmake ../src -DCYCLUS_ROOT_DIR=<cyclus location> 

You should see output like this::

    ...
    ...
    >> -- Configuring done
    >> -- Generating done
    >> -- Build files have been written to: .../onegroupreactor/build
    /onegroupreactor/build$ make onegroupreactor
    >> Scanning dependencies of target onegroupreactor
    ...
    ...
    >> [100%] Building CXX object CMakeFiles/onegroupreactor.dir/SourceFac.cpp.o
    >> Linking CXX executable onegroupreactor
    >> [100%] Built target onegroupreactor

~~~~~~~~~~~~~~~~~~~~~~~~~~~
Installing Your Module
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To allow cyclus to find your module libraries, you must install them within 
the cyclus installation directory. To do so, execute :: 

    .../onegroupreactor/build$ make
    .../onegroupreactor/build$ make install

If the cyclus installation directory is in a system location, you will need to 
execute `sudo make install` instead of `make install`. 
The `Cyclus Homepage`_ has much more detailed guides and information.  If
you intend to develop for *Cyclus*, please visit it to learn more.


.. _`Cyclus Homepage`: http://cyclus.github.com



--------------------------------------------------------------------------
Advanced : A Suggested Developer Workflow
--------------------------------------------------------------------------

If you are working on your module with numerous developers, we recommend a
branching workflow similar to the one described at http://progit.org/book/ch3-4.html.

~~~~~~~~~~~~~~~~~~~
Workflow Notes
~~~~~~~~~~~~~~~~~~~

  * The "develop" branch is how developers will share (generally compilable) progress
    when we are not yet ready for the code to become 'production'.

  * Keep your own "master" and "develop" branches in sync with the upstream repository's
    "master" and "develop" branches. The master branch should always be the 'stable'
    or 'production' release of your module.
    
     - Pull the most recent history from the upstream repository "master"
       and/or "develop" branches before you merge changes into your
       corresponding local branch. 
       For example::

         git checkout develop
         git fetch upstream 
         git pull upstream develop

     - Only merge changes into your "master" or "develop" branch when you
       are ready for those changes to be integrated into the upstream
       repository's corresponding branch. 

  * As you do development on topic branches in your own fork, consider rebasing
    the topic branch onto the "master" and/or "develop"  branches after *pulls* from the upstream
    repository rather than merging the pulled changes into your branch.  This
    will help maintain a more linear (and clean) history.
    For example::

      git checkout [your topic branch]
      git merge develop

  * **Passing Tests**

      - To check that your branch passes the tests, you must build and install your topic 
        branch and then run the OneGroupReactorUnitTestDriver (at the moment, ```make 
        test``` is insufficient). For example ::
      
          mkdir build
          mkdir install
          cd build
          cmake ../src -DCMAKE_INSTALL_PREFIX=../install
          make
          make install
          ../install/onegroupreactor/bin/OneGroupReactorUnitTestDriver

      - There are also a suite of sample input files 
        In addition to the \*UnitTestDriver, a suite of input files can be run and 
        tested using the run_inputs.py script that is configured, built, and installed 
        with your module. It relies on the input files that are part of your Cycstub 
        repository, and only succeeds for input files that are correct (some may have 
        known issues. See the issue list in cyclus for details.) To run the example 
        input files, ::

          python ../install/onegroupreactor/bin/run_inputs.py

  * **Making a Pull Request** 
    
      - When you are ready to move changes from one of your topic branches into the 
        "develop" branch, it must be reviewed and accepted by another 
        developer. 

      - You may want to review this `tutorial <https://help.github.com/articles/using-pull-requests/>`_ 
        before you make a pull request to the develop branch.
        
  * **Reviewing a Pull Request** 

     - Build, install, and test it. If you have added the remote repository as 
       a remote you can check it out and merge it with the current develop 
       branch thusly, ::
       
         git checkout -b remote_name/branch_name
         git merge develop

     - Look over the code. 

        - You may want your code to meet `our style guidelines <http://cyclus.github.com/devdoc/style_guide.html>`_.

        - Make inline review comments concerning improvements. 
      
     - Accept the Pull Request    

        - In general, **every commit** (notice this is not 'every push') to the
          "develop" and "master" branches should compile and pass tests. This
          is guaranteed by using a NON-fast-forward merge during the pull request 
          acceptance process. 
    
        - The green "Merge Pull Request" button does a non-fast-forward merge by 
          default. However, if that button is unavailable, you've made minor 
          local changes to the pulled branch, or you just want to do it from the 
          command line, make sure your merge is a non-fast-forward merge. For example::
          
            git checkout develop
            git merge --no-ff remote_name/branch_name -m "A message of acceptance."

~~~~~~~~~~~~~~~~~~~
See also
~~~~~~~~~~~~~~~~~~~

A good description of a git workflow with good graphics is available at
http://nvie.com/posts/a-successful-git-branching-model/
