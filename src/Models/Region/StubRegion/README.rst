StubRegion
===========

This is a directory intended to hold RegionModel templates. 

That is, it is intended to hold the skeletal stub modules for assistance with 
developing new concrete RegionModels for Cyclus. It also holds elements of a build 
system that facilitates building and installation in tandem with the Cyclus 
building and installation process. 

More information on regions can be found `here <http://cyclus.github.com/devdoc/make-models/region.html/>`_.

What's in this directory?
=========================
- **CMakeLists.txt**, a file directing the build for this directory
- **StubRegion.cpp**, a skeletal RegionModel concrete instance, a template for 
  implementing the expected interface for Regions 
- **StubRegion.h**, a skeletal header file template that defines the
  expected interface for Regions 
- **StubRegionTests.cpp**, a file implementing test templates for the StubRegion
- **StubRegionInput.xml**, an example input file utilizing the StubRegion
- **StubRegion.rng**, a relaxng schema for defining the input parameters of the 
  StubRegion
