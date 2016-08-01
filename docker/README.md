
Each subdirectory contains a dockerfile that does something useful:

* ``cycamore-ci`` is the dockerfile used for running cycamore on a continuous
  integration service.  This dockerfile assumes that the current working
  directory is a cycamore repository - and that version of cycamore is copied
  into the docker container and used for the build.  The dockerfile in the
  cycamore repository root is a symbolic link to this dockerfile.  This
  dockerfile uses the base image ``cyclus/cyclus:latest`` from the docker hub
  repository. This docker container is uploaded as ``cyclus/cycamore:latest`` on
  the docker hub when merging a PR on the develop branch of the Cycamore Github
  repository.

* ``master-ci`` is the dockerfile used for running last stable cycamore version
  a continuous integration service. This dockerfile works exactly as the
  develop-ci one, except that it builds cycamore against the
  ``cyclus/cyclus:stable`` from the docker hub repository. This docker container
  is also uplaoded on the docker hub as the ``cyclus/cycamore:stable`` when
  merging a PR on the master branch of Cycamore Github repository.

* ``release-ci`` contains a script which allows to build the dockerfile require
  to generate a icycamore debian package for the installation of cycamore on
  ubuntu. The script take the major version number of ubuntu as a parameter. It
  builds Cycamore on the corresponding Ubuntu-LTS version, after installing all
  Cycamore dependencies -including Cyclus- and generates the Cycamore debian
  installation package that need to be extracted from the docker container
  (tested on Ubuntu 14.04 & 16.04)


