###################################################
Cycamore : The CYClus Additional MOdules REpository
###################################################

Additional modules for the Cyclus nuclear fuel cycle simulator from the
University of Wisconsin - Madison are intended to support innovative
fuel cycle simulations with the Cyclus fuel cycle simulator.

This README is intended primarily for those who intend to contribute to the
development of Cycamore archetypes.  If you are interested in Cyclus as a user
or in developing your own Cyclus archetyes, you may want to consult `Getting
Started with Cyclus <http://fuelcycle.org/user/install.html>`_.

This README provides basic information about:
 - the dependency required by Cycamore
 - installation of Cycamore from the command line
 - how to run Cycamore unit tests

Although Cycamore is meant to enable a basic fuel cycle simulation, you may want
more functionality than it offers.  Third party modules can also be installed
(or developed) with additional facilities.  Please visit the Cyclus website for
a list of `contributed modules <http://fuelcycle.org/user/index.html#third-party-archetypes>`_.

- **For general information about Cyclus, visit the**  `Cyclus Homepage`_,

- **For detailed installation instructions, visit the**
  `INSTALLATION Guide <INSTALL.rst>`_,

- **To see user and developer documentation for this code, please visit
  the** `Users Guide <http://fuelcycle.org/user/index.html>`_,

- **If you would like to contribute to Cycamore, please check our**
  `Contribution Guidelines <https://github.com/cyclus/cyclus/blob/main/CONTRIBUTING.rst>`_.


.. contents:: Table of Contents


************
Dependencies
************

Cycamore's only dependency is the Cyclus Core.

====================   ==================
Package                Minimum Version
====================   ==================
`Cyclus`               1.5
====================   ==================

There are a number of ways to install the Cyclus core:

- To install from source code, see the `Cyclus Core repository
  <http://github.com/cyclus/cyclus>`_

- To install from a binary distribution, see the instructions for
  `Installing Cyclus from Binaries <DEPENDENCIES.rst>`_

******************************
Quick Cycamore Installation
******************************

Assuming you have the dependencies installed correctly, installing Cyclus using
github is fairly straightforward:

- Clone the Cyclus Repo: ``git clone https://github.com/cyclus/cycamore.git``,

- to install Cyclus locally (in ``~/.local/``) just run: ``python3 install.py``
  from cycamore folder,

- finally, add the following Cyclus installation path (``~/.local/cyclus``) to
  the **bottom** on your ``$PATH``.

For more detailed installation procedure, and/or custom installation please
refer to the `INSTALLATION guide <INSTALL.rst>`_.


******************************
Running Tests
******************************

Installing Cycamore will also install a test driver (i.e., an executable of all of
our tests). You can run the tests yourself via:

.. code-block:: bash

    $ cycamore_unit_tests

******************************
Contributing
******************************

We happily welcome new developers into the Cyclus Developer Team. If you are willing
to contribute into Cyclus, please follow this procedure:

#. Fork Cycamore repository,

#. Create a working branch on you fork from the ``main`` branch,

#. Implement your modification of the Cycamore source code,

#. Submit a Pull request into ``cycamore/main`` branch,

#. Wait for reviews/merge (the Puller cannot be the Merger).

You may also want to read our `Contribution Guidelines <CONTRIBUTING.rst>`_.

.. _`CMake`: https://cmake.org
.. _`Cyclus Homepage`: http://fuelcycle.org/
.. _`Cyclus User Guide`: http://fuelcycle.org/user/index.html
.. _`Cyclus repo`: https://github.com/cyclus/cyclus
.. _`Cycamore Repo`: https://github.com/cyclus/cycamore
.. _`INSTALL`: INSTALL.rst
.. _`CONTRIBUTING`: CONTRIBUTING.rst

