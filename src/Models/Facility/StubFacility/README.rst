StubFacility
=============

This is a directory intended to hold FacilityModel templates. 

That is, it is intended to hold the skeletal stub modules for assistance with 
developing new concrete FacilityModels for Cyclus. It also holds elements of a build 
siestem that facilitates building and installation in tandem with the Cyclus 
building and installation process. 

More information on facilities can be found `here <http://cyclus.github.com/devdoc/make-models/facility.html/>`_.

What's in this directory?
=========================
- **CMakeLists.txt**, a file directing the build for this directory
- **StubFacility.cpp**, a skeletal FacilityModel concrete instance, a template for 
  implementing the expected interface for Facilitys 
- **StubFacility.h**, a skeletal header file template that defines the
  expected interface for Facilitys 
- **StubFacilityTests.cpp**, a file implementing test templates for the StubFacility
- **StubFacilityInput.xml**, an example input file utilizing the StubFacility
- **StubFacility.rng**, a relaxng schema for defining the input parameters of the 
  StubFacility
