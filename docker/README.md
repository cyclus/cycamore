
Each subdirectory contains a dockerfile that does something useful:

* ``cycamore-ci`` is the dockerfile used for running cycamore on a continuous
  integration service.  This dockerfile assumes that the current working
  directory is a cycamore repository - and that version of cycamore is copied
  into the docker container and used for the build.  The dockerfile in the
  cycamore repository root is a symbolic link to this dockerfile.  This
  dockerfile uses the base image ``cyclus/cyclus`` from the docker hub
  repository.

* ``cloudlus-worker``

