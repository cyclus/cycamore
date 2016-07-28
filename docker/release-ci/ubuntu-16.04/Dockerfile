FROM ubuntu:16.04

RUN echo "1"

RUN apt-get -y --force-yes update

RUN apt-get install -y --force-yes \
    cmake \
    make \
    libboost-all-dev \
    libxml2-dev \
    libxml++2.6-dev \
    libsqlite3-dev \
    libhdf5-serial-dev \
    libbz2-dev \
    coinor-libcbc-dev \
    coinor-libcoinutils-dev \
    coinor-libosi-dev \
    coinor-libclp-dev \
    coinor-libcgl-dev \
    libblas-dev \
    liblapack-dev \
    g++ \
    libgoogle-perftools-dev \
    git \
    python \
    python-tables \
    python-numpy \
    python-nose

# build cyclus deb
RUN git clone https://github.com/cyclus/cyclus.git /cyclus
WORKDIR cyclus
RUN git fetch
RUN git checkout develop
RUN ./install.py --prefix="/usr/local"

COPY . /cycamore 
WORKDIR cycamore
RUN mkdir -p deb
WORKDIR /cycamore/deb
RUN cmake -DCMAKE_INSTALL_PREFIX=/usr -DCPACK_SYSTEM_NAME=ubuntu.16.04 ..
RUN cpack -v ..

