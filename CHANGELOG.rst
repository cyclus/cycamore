=================
cyclus Change Log
=================

.. current developments

v1.5.5
====================
**Changed:**

* A reactor will now decommission itself if it is retired and the decomission requirement is met.

v1.5.4
====================

**Added:**

* RecordTimeSeries has been added to the several archetypes; Reactor, Source, Sink,
  FuelFab, Separations, and Storage. This change was made to allow these agents to
  interact with the d3ploy archetypes. 
* Added unit tests for Cycamore archetypes with Position toolkit.

* Record function for Cycamore archetypes' coordinates in Sqlite Output.

**Changed:** 

- All cycamore archetypes have been edited to now include Cyclus::toolkit::Position.




v1.5.3
====================

**Changed:**

* Many build system improvements, including making COIN optional.




