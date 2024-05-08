ARG pkg_mgr=apt
ARG ubuntu_version=22.04
ARG cyclus_tag=latest

FROM ghcr.io/cyclus/cyclus_${ubuntu_version}_${pkg_mgr}/cyclus:${cyclus_tag} as cycamore
ARG make_cores=2

COPY . /cycamore
WORKDIR /cycamore

RUN python3 install.py -j ${make_cores} --allow-milps

FROM cycamore as deb-generation
WORKDIR /cycamore/build
RUN make package

FROM scratch as deb-package
COPY --from=deb-generation /cycamore/build/cycamore*.deb /

FROM cycamore as cycamore-test
RUN cycamore_unit_tests

FROM cycamore-test as cycamore-pytest
RUN cd tests && python3 -m pytest
