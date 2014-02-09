Cyclus/Cycamore Regression Tests
================================

Dependencies
------------

* PyTables 3.0.0 or higher
* Python 2.7 (*only*)

Running Tests
-------------

To run the regression tests from the cycamore/tests:

.. code-block:: bash

  $ nosetests

New Releases
------------

On each new release (major, minor, micro), the release manager is responsible
for updating the regression test databases updated.

First, generate the new databases:

.. code-block:: bash

  $ python refs.py gen

Next, rename the databases:

.. code-block:: bash

  $ rename 's/^[^_]*_[^_]*_(.*)/<cyclus version>_<cycamore version>_$1/' *.h5

where

* <cyclus version> is replaced by the current version tag name for cyclus
  (e.g. v0.1)
* <cycamore version> is replaced by the current version tag name for cycamore
  (e.g. v0.1)

so that, using the above examples, the command is

.. code-block:: bash

  $ rename 's/^[^_]*_[^_]*_(.*)/v0.1_v0.1_$1/' *.h5

At this point, you will need to get a credentials file, which exists in the
metadata document in the Cyclus-CI shared folder on Google drive. If you have
questions, please email cyclus-ci@googlegroups.com. The file must be named
`rs.cred`.

Now, update (add) them on the regression test server

.. code-block:: bash

  $ python refs.py add *.h5

Finally, feel free to clean up after yourself

.. code-block:: bash

  $ rm *.h5

Nondeterminisitic Analysis
==========================

An `analysis` python module can assist in analyzing the determinism of Cyclus
output. It does so by running the regression tests some number of times and
analyzing the frequency of nondeterminism of output tables and columns within
those tables. See the module's help: 

.. code-block:: python

  $ python analysis.py -h
