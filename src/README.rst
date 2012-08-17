SRC
=======

This is a directory intended to hold the module source files.
It mimics the installation hierarchy in cyclus. 

That is, it is the top level source directory for module building and is intended to 
hold the skeletal stub modules for assistance with 
developing new concrete model for Cyclus. It also holds a build system that facilitates 
building and installation in tandem with the Cyclus building and installation 
process. 

What's in this directory?
=========================
- CMake, a directory that holds build system configuration files
- Models, a directory that holds concrete model templates, called Stubs.
- Testing, a directory that holds test templates for the model templates. 
- doc, a directory that holds configuration files for the doxygen documentation
- post_install, a directory that holds a post installation script 
