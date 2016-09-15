###################################################
Cycamore : The CYClus Additional MOdules REpository
###################################################

Additional modules for the Cyclus nuclear fuel cycle simulator from the
University of Wisconsin - Madison are intended to be support innovative
fuel cycle simulations with the Cyclus fuel cycle simulator.

Cycamore require Cyclus. If you don't have installed Cyclus yet please checkout
the `Cyclus website <http://fuelcycle.org/>`_ or the `Cyclus repository <https://github.com/cyclus/cyclus>`_.

Third party modules can also be installed (or developed) with additional
facilities.  Please visit the Cyclus website for a list of contributed modules.

- **For general information about Cyclus, visit the**  `Cyclus Homepage`_,

- **For detailed installation instructions, visit the**
  `INSTALLATION Guide <INSTALL.rst>`_,

- **To see user and developer documentation for this code, please visit
  the** `Users Guide <http://fuelcycle.org/user/index.html>`_, 

- **If you would like to contribute to Cyclus, please check our** 
  `Contribution Guidelines <CONTRIBUTING.rst>`_.


.. contents:: Table of Contents


******************************
Dependencies
******************************

In order to facilitate future compatibility with multiple platforms,
Cyclus is built using `CMake`_. A full list of the Cycamore package
dependencies is shown below:

====================   ==================
Package                Minimum Version
====================   ==================
`Cyclus`               1.4
====================   ==================


You can find the Cyclus Core which is require to use the Cycamore additionnal
Modelus. You can download Cyclus from the git repository.
(`GitHub Cyclus Repository <https://github.com/cyclus/cyclus>`_):

- Clone the Cyclus Repo: ``git clone
  https://github.com/cyclus/cyclus.git``,

- Install Cyclus  in ``~/.local/``:
  ``python install.py`` from the Cyclus folder.

******************************
Quick Cycamore Installation
******************************

Assuming you have the dependencies installed correctly, installing Cyclus using
github is fairly straightforward:

- Clone the Cyclus Repo: ``git clone https://github.com/cyclus/cycamore.git``,

- to install Cyclus locally (in ``~/.local/``) just run: ``python install.py``
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

#. Create a working branch on you fork from the `develop` branch,

#. Implement your modification of the Cycamore source code,

#. Submit a Pull request into `Cycamore/develop` branch,

#. Wait for reviews/merge (the Puller cannot be the Merger).

You may also want to read our `Contribution Guidelines <CONTRIBUTING.rst>`_.

.. _`CMake`: https://cmake.org
.. _`Cyclus Homepage`: http://fuelcycle.org/
.. _`Cyclus User Guide`: http://fuelcycle.org/user/index.html
.. _`Cyclus repo`: https://github.com/cyclus/cyclus
.. _`Cycamore Repo`: https://github.com/cyclus/cycamore
.. _`INSTALL`: INSTALL.rst
.. _`CONTRIBUTING`: CONTRIBUTING.rst

