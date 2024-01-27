#!/bin/bash
export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10

set -e 
set -x
mkdir -p build
cd build
cmake ..
cd ..
make -j4 -C build 
make install