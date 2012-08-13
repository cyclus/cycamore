StubMarket
============

This is a directory intended to hold MarketModel templates. 

That is, it is intended to hold the skeletal stub modules for assistance with 
developing new concrete MarketModels for Cyclus. It also holds elements of a build 
system that facilitates building and installation in tandem with the Cyclus 
building and installation process. 

More information on markets can be found `here <http://cyclus.github.com/devdoc/make-models/market.html/>`_.

What's in this directory?
=========================
- **CMakeLists.txt**, a file directing the build for this directory
- **StubMarket.cpp**, a skeletal MarketModel concrete instance, a template for 
  implementing the expected interface for Markets 
- **StubMarket.h**, a skeletal header file template that defines the
  expected interface for Markets 
- **StubMarketTests.cpp**, a file implementing test templates for the StubMarket
- **StubMarketInput.xml**, an example input file utilizing the StubMarket
- **StubMarket.rng**, a relaxng schema for defining the input parameters of the 
  StubMarket
