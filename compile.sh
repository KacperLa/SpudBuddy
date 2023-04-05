#!/bin/bash

set -e 
set -x
mkdir -p build
cd build
cmake ..
cd ..
make -j4 -C build