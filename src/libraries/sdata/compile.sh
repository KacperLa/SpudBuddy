#!/bin/bash
export CC=/usr/bin/gcc-11
export CXX=/usr/bin/g++-11

set -e 
set -x
mkdir -p build
cd build
cmake ..
cd ..
make -j4 -C build
cd build
sudo make install


# -- Installing: /usr/local/lib/libsdata.so
# -- Installing: /usr/local/include/sdata.h
# -- Installing: /usr/local/lib/pkgconfig/sdata.pc