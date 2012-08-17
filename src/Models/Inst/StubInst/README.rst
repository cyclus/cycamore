StubInst
=========

This is a directory intended to hold InstModel templates. 

That is, it is intended to hold the skeletal stub modules for assistance with 
developing new concrete Institution Models for Cyclus. It also holds elements of a build 
system that facilitates building and installation in tandem with the Cyclus 
building and installation process. 

More information on institutions can be found `here <http://cyclus.github.com/devdoc/make-models/inst.html/>`_.

What's in this directory?
=========================
- **CMakeLists.txt**, a file directing the build for this directory
- **StubInst.cpp**, a skeletal InstModel concrete instance, a template for 
  implementing the expected interface for Insts 
- **StubInst.h**, a skeletal header file template that defines the
  expected interface for Insts 
- **StubInstTests.cpp**, a file implementing test templates for the StubInst
- **StubInstInput.xml**, an example input file utilizing the StubInst
- **StubInst.rng**, a relaxng schema for defining the input parameters of the 
  StubInst
