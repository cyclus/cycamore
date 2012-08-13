Models
=======

This is a directory intended to hold model templates of 
various types. It mimics the installation hierarchy in cyclus. 

That is, it is intended to hold the skeletal stub modules for assistance with 
developing new concrete model for Cyclus. It also holds elements of a build 
system that facilitates building and installation in tandem with the Cyclus 
building and installation process. 

What's in this directory?
=========================
- CMakeLists.txt, a file directing the build for this directory
- Converter, a directory that holds a skeletal 
  StubConverter.
- Facility, a directory that holds a skeletal StubFacility
  `more <http://cyclus.github.com/devdoc/make-models/facility.html/>`...
- Inst, a directory that holds a skeletal StubInst
  `more <http://cyclus.github.com/devdoc/make-models/inst.html/>`...
- Market, a directory that holds a skeletal StubMarket
  `more <http://cyclus.github.com/devdoc/make-models/market.html/>`...
- Region, a directory that holds a skeletal StubRegion
  `more <http://cyclus.github.com/devdoc/make-models/region.html/>`...
- Stub, a directory that holds a skeletal StubStub (for making classes of Model 
  that do not yet exist )
- StubComm, a directory that holds a skeletal StubStubComm (for making classes 
  of Communicator Model that do not exist yet)
