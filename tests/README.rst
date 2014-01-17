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

.. code-block:: bash

  $ python refs.py

