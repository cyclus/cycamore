Cycamore (Cyclus) Sample Inputs: Minimal Input
===============================================

This group of input files are designed to demonstrate the smallest possible
system that engages all of Cyclus' core capabilities.

Single Source <--> Single Sink
-------------------------------

This example uses a single source facility and a single sink facility, both
operating in the same region and institution.



Understanding the input file
+++++++++++++++++++++++++++++

`control` block
~~~~~~~~~~~~~~~
This simulation starts in January 2000 and proceeds for 10 months.

`acrhetypes` block
~~~~~~~~~~~~~~~~~~
This simulation uses four archetypes:
* the `Source` archetype from the `cycamore` library
* the `Sink` archetype from the `cycamore` library
* the `NullRegion` archetype from the Cyclus core `agents` library
* the `NullInst` archetype from the Cyclus core `agents` library

`facility` block
~~~~~~~~~~~~~~~~~
This simulation defines two facility prototypes:
* a prototype named `Sink` based on the `Sink` archetype, that defines a
  single output commodity named `commodity`
* a prototype named `Source` based on the `Source` archetype, that defines a
  single input commodity named `commodity`

`region` block
~~~~~~~~~~~~~~
This simulation defines only one region based on the `NullRegion` archetype,
and named `SingleRegion`.  This archetype has no configuration other than a
single institution.

`institution` block
~~~~~~~~~~~~~~~~~~~
This simulation defines only one region based on the `NullInst` archetype, and
named `SingleInstitution`.  All institutions allow an `initialfacilitylist`,
but otherwise this archetype has no configuration.
