#####################
Cycamore Dependencies
#####################


To see user and developer documentation for this code, please visit
the `Cyclus Homepage`_.

.. contents:: Table of Contents
   :depth: 2

************
Dependencies
************

====================   ==================
Package                Minimum Version
====================   ==================
`Cyclus`               1.4
====================   ==================


***********************
Installing Dependencies
***********************

 The only Cycamore dependency is Cyclus. We support 3 main ways to install
 Cyclus.

Installing Cyclus using Deb package:
------------------------------------

This installation procedure assumes that you are using Ubuntu (LTS) 14.04 or
16.04. This method has only been tested on those Ubuntu versions. This
installation procedure also assumes that you have root access to you computer.

#.  Install |Cyclus| dependencies:

    .. code-block:: bash 

       $ sudo apt-get install libtcmalloc-minimal4 libboost-filesystem-dev libboost-program-options-dev libboost-serialization-dev libhdf5-dev libxml++2.6-dev coinor-libcbc-dev
  

    WARNING: This dependency list is ONLY correct for the debian binary
    installation, additional dependencies are required for to install from source.

#.  Download the |Cyclus| Debian installation package corresponding to your
    Ubuntu version (`14.04
    <http://dory.fuelcycle.org:4848/cyclus_1.4.0_14dbaed_ubuntu.14.04.deb>`_ or
    `16.04
    <http://dory.fuelcycle.org:4848/cyclus_1.4.0_14dbaed_ubuntu.16.04.deb>`_).

#.  Install the package by running:

    .. code-block:: bash 

       $ sudo dpkg -i CYCLUS_DEB_PACKAGE_MANE.deb

    where the actual filename is inserted in place of 'CYCLUS_DEB_PACKAGE_MANE.deb'.

Installing Cyclus using Conda:
------------------------------

1.  a.  If you don't have Conda, follow the `install conda documentation <http://fuelcycle.org/user/install_conda.html>`_ to prepare it for cyclus, then skip to step 2.

    b.  If you already have conda installed, installing |Cyclus| is even easier.
        You simply need to make sure that conda-forge is part of your channels.
        Please edit the ``channels`` section of your ``~/.condarc`` to include
        the ``conda-forge`` channel.  For example,

        .. code-block:: yaml

            channels:
              - conda-forge
              - defaults
      
2.  Once this is done, install |Cyclus|  and Cycamore with the following comand.

    .. code-block:: bash

       $ conda install --yes cyclus



Installing Cyclus from the source:
----------------------------------


Get Cyclus source:
==================

From the Git Repo
.................

.. code-block:: bash

  git clone https://github.com/cyclus/cyclus .
  git fetch
  git checkout master

 
From a Tarball
..............

Download the most recent stable version of Cyclus source (either .zip or .gz):
  - `cyclus.1.3.1.zip  <https://github.com/cyclus/cyclus/archive/1.3.1.zip>`_
  - `cyclus.1.3.1.tar.gz  <https://github.com/cyclus/cyclus/archive/1.3.1.tar.gz>`_

(`Previous versions <https://github.com/cyclus/cyclus/releases>`_ of |Cyclus|)
  

Compile Cyclus
==============


Run the install script:

.. code-block:: bash
  
  python install.py


If you successfully followed the instruction above cyclus binary have been
generated and be placed in the  ``.local/`` in your home directory. 
You need to had ``~/.local/bin`` to the bottom of your ``$PATH``:

.. code-block:: bash
  
  echo 'export PATH="$HOME/.local/bin:$PATH' >> .bashrc



  .. _`Cyclus Homepage`: http://fuelcycle.org/
.. _`Cyclus User Guide`: http://fuelcycle.org/user/index.html
.. _`Cyclus repo`: https://github.com/cyclus/cyclus
.. _`Cycamore Repo`: https://github.com/cyclus/cycamore

