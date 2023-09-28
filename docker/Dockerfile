ARG pkg_mgr=apt
ARG ubuntu_version=22.04
ARG cyclus_tag=latest
# eventually will include an ARG for main vs dev tag since that should be in the
# workflow matrix

FROM ghcr.io/cyclus/cyclus_${ubuntu_version}_${pkg_mgr}/cyclus:${cyclus_tag} as cycamore
ARG make_cores=2

COPY . /cycamore
WORKDIR /cycamore

RUN python install.py -j ${make_cores}
FROM cycamore as cycamore-test
RUN cycamore_unit_tests

FROM cycamore as cycamore-pytest
RUN apt install -y python3-nose
RUN cd tests && nosetests3
