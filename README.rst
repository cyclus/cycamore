.. _hello_world:

StreamBlender
==============

This facility blends resources into a goal resource based on preferences and a 
goal recipe. It was intended to be used as a fuel fabrication facility, but has 
been kept very general in order to allow users to employ it for many purposes.

Usage with Cyclus
-----------------

If you want to use this model with cyclus, then you should first install 
cyclus. The information for that can be found at fuelcycle.org. It's pretty 
quick, so go ahead and just install it. I'll wait here. 

Installing StreamBlender
------------------------

Ok, are you done? Once you have Cyclus installed, you can use the 
StreamBlender. Its installation is just like the installation of CycStub 
(because it was built based on CycStub).

Getting StreamBlender
.....................

First, you need to get the ``streamblender`` code.  St
You can grab StreamBlender either by using git to 
`clone the repository <https://github.com/katyhuff/streamblender.git>`_ or by 
`downloading the zip file <https://github.com/katyhuff/streamblender/archive/develop.zip>`_.

**Getting streamblender via git:**

.. code-block:: bash

    $ git clone https://github.com/katyhuff/streamblender.git streamblender
    $ cd streamblender

**Getting streamblender via zip:**

.. code-block:: bash

    $ curl -L https://api.github.com/repos/katyhuff/streamblender/zipball > streamblender.zip
    $ unzip streamblender.zip
    $ mv katyhuff-streamblender-* streamblender
    $ cd streamblender

------------

Now, you can use the StreamBlender model by including it in an xml input file 
that you run with Cyclus, along with other modules. 
