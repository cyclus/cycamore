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
- **CMakeLists.txt**, a file directing the build for this directory
- **Converter**, a directory that holds a skeletal 
  StubConverter.
- **Facility**, a directory that holds a skeletal StubFacility.
  `more on facilities <http://cyclus.github.com/devdoc/make-models/facility.html/>`_
- **Inst**, a directory that holds a skeletal StubInst
  `more on institutions <http://cyclus.github.com/devdoc/make-models/inst.html/>`_
- **Market**, a directory that holds a skeletal StubMarket
  `more on markets <http://cyclus.github.com/devdoc/make-models/market.html/>`_
- **Region**, a directory that holds a skeletal StubRegion
  `more on regions <http://cyclus.github.com/devdoc/make-models/region.html/>`_
- **Stub**, a directory that holds a skeletal StubStub (for making classes of Model 
  that do not yet exist )
- **StubComm**, a directory that holds a skeletal StubStubComm (for making classes 
  of Communicator Model that do not exist yet)
