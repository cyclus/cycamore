ARG pkg_mgr=apt
ARG ubuntu_version=22.04

FROM ghcr.io/cyclus/cyclus_${ubuntu_version}_${pkg_mgr}/cyclus as cycamore
ARG make_cores=2

COPY . /cycamore
WORKDIR /cycamore

RUN python install.py -j ${make_cores} --build-type=Release --core-version 99999.99999

FROM cycamore as cycamore-test
RUN cycamore_unit_tests

FROM cycamore as cycamore-pytest
RUN apt install -y python3-nose
RUN cd tests && nosetests3
