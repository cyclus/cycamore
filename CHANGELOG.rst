=================
cycamore Change Log
=================

.. current developments
**Added:**

* Downstream testing in CI workflows (#573, #580, #582, #583)
* GitHub workflow for publishing images and debian packages on release (#573, #582, #583, #593)
* GitHub workflows for building/testing on a PR and push to `main` (#549, #564, #573, #582, #583, #590)
* Add functionality for random behavior on the size (#550) and frequency (#565) of a sink
* GitHub workflow to check that the CHANGELOG has been updated (#562) 
* Added inventory policies to Storage through the material buy policy (#574, #588)

**Changed:** 

* Updated build procedure to use newer versions of packages in 2023 (#549)
* Added active/dormant and request size variation from buy policy to Storage (#546, #568, #586, #587)
* Update build procedure to force a rebuild when a test file is changed (#584)
* Define the version number in `CMakeLists.txt` and rely on CMake to propagate the version throughout the code (#589)
* Update version numbers in documentation and fix references to `master` branch (#591)

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




