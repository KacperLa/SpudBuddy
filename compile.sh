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

# Copy python lib to server folder 
cp build/lib/SDataLib.cpython-39-aarch64-linux-gnu.so server/SDataLib.so