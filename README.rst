.. _hello_world:

Separation Matrix
=================

This facility separates materials into elemental streams based on an efficiency matrix.
It was intended to be used as an aqueous reprocessing facility, but has 
been kept very general in order to allow users to employ it for many purposes.

By describing the separations process as a simple matrix of efficiencies, a
material stream transformation can be conducted. The specific process chemistry
for the separation at hand is treated as elemental, as representative of a
non-laser separations process. The efficiencies must be defined to transform an
incoming composition vector $I$ with $N$ constituent amounts, $I_n$ to an
outgoing set of $M$ streams, $E_m$. The efficiency matrix $\eta$ is therefore
an $N\times M$ matrix of efficiencies. The matrix of separation efficiencies
has a default value: the identity matrix of size $N\times N$. In this context,
the identity matrix represents complete and perfect elemental separation
without losses. 

\[
  \left[
    \begin{array}{c c c c c c c}
      \eta_{11} & . & . & . & . & . & \eta_{1M} \\
      \eta_{21} & . & . & . & . & . & \eta_{2M} \\
      . & . & . & . & . & . & . \\
      . & . & . & . & . & . & . \\
      . & . & . & . & . & . & . \\
      \eta_{N1} & .  & . & . & . & . & \eta_{NM} \\
    \end{array}
    \right]
  \left[
    \begin{array}{c}
      I_1\\
      I_2\\
      . \\
      . \\
      . \\
      I_N\\
    \end{array}
    \right]
    =
    \left[
      \begin{array}{ c }
        E_1\\
        E_2\\
        .\\
        .\\
        .\\
        E_M\\
      \end{array}
      \right]
\]

For realistic separations, the user is expected to produce an efficiency 
matrix representing the separations technology of interest to them. 
By requesting the feedstock from the 
appropriate markets, the facility acquires an unseparated feedstock stream. 
Based on the input parameters  in Table \ref{tab:sepmatrix}, the separations 
process proceeds within the timesteps and other constraints of the simulation. 

Thereafter, separated streams as well as a stream of losses are offered the 
appropriate markets for consumption by other facilities. In the transition 
scenario at hand, the StreamBlender fuel fabrication facility purchases the 
streams it desires in order to produce SFR fuel. 

Usage with Cyclus
-----------------

If you want to use this model with cyclus, then you should first install 
cyclus. The information for that can be found at fuelcycle.org. It's pretty 
quick, so go ahead and just install it. I'll wait here. 

Installing SeparationMatrix
----------------------------

Ok, are you done? Once you have Cyclus installed, you can use the 
SeparationMatrix. Its installation is just like the installation of CycStub 
(because it was built based on CycStub).

Getting SeparationMatrix
........................

First, you need to get the ``separationmatrix`` code.  St
You can grab SeparationMatrix either by using git to 
`clone the repository <https://github.com/katyhuff/separationmatrix.git>`_ or by 
`downloading the zip file <https://github.com/katyhuff/separationmatrix/archive/develop.zip>`_.

**Getting separationmatrix via git:**

.. code-block:: bash

    $ git clone https://github.com/katyhuff/separationmatrix.git separationmatrix
    $ cd separationmatrix

**Getting separationmatrix via zip:**

.. code-block:: bash

    $ curl -L https://api.github.com/repos/katyhuff/separationmatrix/zipball > separationmatrix.zip
    $ unzip separationmatrix.zip
    $ mv katyhuff-separationmatrix-* separationmatrix
    $ cd separationmatrix

------------

Now, you can use the SeparationMatrix model by including it in an xml input file 
that you run with Cyclus, along with other modules. 
