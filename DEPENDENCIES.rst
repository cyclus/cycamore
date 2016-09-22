###################################################
Cycamore : The CYClus Additional MOdules REpository
###################################################


.. contents:: Table of Contents
   :depth: 2

******************************
Dependencies
******************************

In order to facilitate future compatibility with multiple platforms,
Cyclus is built using `CMake`_. A full list of the Cyclus package
dependencies is shown below:

====================   ==================
Package                Minimum Version
====================   ==================
`CMake`                2.8
`boost`                1.46.1
`libxml2`              2
`Cyclus`               1.4
====================   ==================

******************************
Installing Dependencies
******************************

Cyclus dependencies can either be installed via an operating system's package
manager or via Conda.

Via Conda
---------

*Note, at present, Conda dependency downloading is only supported for Linux*

A Conda installation may be the most straightforward for a new developer. These
instructions will be similar to those shown for Cyclus users `conda
installation <http://fuelcycle.org/user/install.html>`_.

#. Download the appropriate `miniconda installer <http://conda.pydata.org/miniconda.html>`_

#. Install miniconda

    .. code-block:: console

        $ bash Miniconda-3.5.2-Linux-x86_64.sh -b -p ~/miniconda

#. Add the following line to your ``.bashrc`` (``.bash_profile`` for Mac users):

    ``export PATH-"${HOME}/miniconda/bin:${PATH}"``

#. Install the dependencies

    .. code-block:: console

        $ conda install cyclus-deps

Installing Dependencies (Linux and Unix)
----------------------------------------

This guide assumes that the user has root access (to issue sudo commands) and
access to a package manager or has some other suitable method of automatically
installing established libraries. This process was tested using a fresh install
of Ubuntu 12.10 using apt-get as the package manager (scroll down further for
Mac OSX instructions).

The command to install a dependency takes the form of:

.. code-block:: bash

  sudo apt-get install package

where "package" is replaced by the correct package name. The minimal list of
required library package names is:

#. make
#. cmake
#. libboost-all-dev (see note below)
#. libxml2-dev
#. libxml++2.6-dev
#. libsqlite3-dev
#. libhdf5-serial-dev
#. libbz2-dev
#. coinor-libcbc-dev
#. coinor-libcoinutils-dev
#. coinor-libosi-dev
#. coinor-libclp-dev
#. coinor-libcgl-dev

and (optionally):

#. doxygen
#. g++
#. libblas-dev
#. liblapack-dev
#. libgoogle-perftools-dev

For example, in order to install libxml++ (and libxml2) on your system, type:

.. code-block:: bash

  sudo apt-get install libxml++2.6-dev

If you'd prefer to copy/paste, the following line will install all *Cyclus*
dependencies:

.. code-block:: bash

   sudo apt-get install -y cmake make libboost-all-dev libxml2-dev libxml++2.6-dev libsqlite3-dev libhdf5-serial-dev libbz2-dev coinor-libcbc-dev coinor-libcoinutils-dev coinor-libosi-dev coinor-libclp-dev coinor-libcgl-dev libblas-dev liblapack-dev g++ libgoogle-perftools-dev

Boost Note
^^^^^^^^^^

The `libboost-all-dev` used above will install the entire Boost library, which
is not strictly needed. We currently depend on a small subset of the Boost
libraries:

#. libboost-program-options-dev
#. libboost-system-dev
#. libboost-filesystem-dev

However, it is possible (likely) that additional Boost libraries will be used
because they are an industry standard. Accordingly, we suggest simply installing
`libboost-all-dev` to limit any headaches due to possible dependency additions
in the future.

Installing Dependencies (Mac OSX)
----------------------------------------

Cyclus archetype development is not fully supported on Mac.  Nonetheless,
because there are some use cases which require installation from source, we have
compiled a list of instructions that should be successful.  Use a Mac platform
at your own risk, we strongly recommend sticking to Linux for development.

This guide assumes that the user has root access (to issue sudo commands) and
access to a package manager or has some other suitable method of automatically
installing established libraries. This process was tested using a fresh install
of Yosemite 10.10.2 using macports as the package manager.  Macports installs
packages in /opt/local.  If installing to a different location,
(i.e. /usr/local) change paths in the following instructions accordingly. If
you use homebrew, try the following instructions with brew commands in place of
the port commands.

The command to install a dependency takes the form of:

.. code-block:: bash

  sudo port install package

where "package" is replaced by the correct package name. The minimal list of
required library package names is:

#. cmake
#. boost
#. hdf5
#. libxml2
#. libxmlxx2
#. sqlite3
#. doxygen
#. glibmm

Then install Coin-Cbc from source. They can be downloaded to any directory on
your computer:
   
**Coin-Cbc**: Download and build using the svn command in the terminal:
   
.. code-block:: bash

  svn co https://projects.coin-or.org/svn/Cbc/stable/2.8 Coin-Cbc
  cd Coin-Cbc/
  mkdir build
  cd build/
  ../configure --prefix=/opt/local
  make
  sudo make install


Finally, update your path and the following environment variables in your
~/.profile (or ~/.bashrc ) file:

.. code-block:: bash

  export DYLD_FALLBACK_LIBRARY_PATH=/opt/local/lib:/opt/local:$DYLD_FALLBACK_LIBRARY_PATH

  export CMAKE_MODULE_PATH=/opt/local/include:$CMAKE_MODULE_PATH
  export CMAKE_PREFIX_PATH=/opt/local:$CMAKE_PREFIX_PATH

  # add to PATH:
  export PATH=${HDF5_DIR}:/opt/local/bin:${HOME}/.local/bin:$PATH



.. _`Cyclus Homepage`: http://fuelcycle.org/
.. _`Cyclus User Guide`: http://fuelcycle.org/user/index.html
.. _`Cyclus repo`: https://github.com/cyclus/cyclus
.. _`Cycamore Repo`: https://github.com/cyclus/cycamore

