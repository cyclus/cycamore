StubConverter
===============

This is a directory intended to hold ConverterModel templates. 

That is, it is intended to hold the skeletal stub modules for assistance with 
developing new concrete ConverterModels for Cyclus. It also holds elements of a build 
system that facilitates building and installation in tandem with the Cyclus 
building and installation process. 

What's in this directory?
=========================
- **CMakeLists.txt**, a file directing the build for this directory
- **StubConverter.cpp**, a skeletal ConverterModel concrete instance, a template for 
  implementing the expected interface for Converters 
- **StubConverter.h**, a skeletal header file template that defines the
  expected interface for Converters 
- **StubConverterTests.cpp**, a file implementing test templates for the StubConverter
- **StubConverterInput.xml**, an example input file utilizing the StubConverter
- **StubConverter.rng**, a relaxng schema for defining the input parameters of the 
  StubConverter
