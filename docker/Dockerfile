ARG pkg_mgr=apt
ARG ubuntu_version=22.04
ARG cyclus_tag=stable

FROM ghcr.io/cyclus/cyclus_${ubuntu_version}_${pkg_mgr}/cyclus:${cyclus_tag} as cycamore
ARG make_cores=2

COPY . /cycamore
WORKDIR /cycamore

RUN python install.py -j ${make_cores} --build-type=Release --core-version 999999.999999

FROM cycamore as cycamore-test
RUN cycamore_unit_tests

FROM cycamore-test as cycamore-pytest
RUN cd tests && python -m pytest