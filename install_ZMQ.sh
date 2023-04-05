!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
cd "$parent_path"

cd "$parent_path"/src/libraries/libzmq
mkdir build
cd build
cmake .. -DCPPZMQ_BUILD_TESTS=OFF
sudo make -j4 install

#git submodule update --init --recursive
cd "$parent_path"src/libraries/cppzmq
rm -fr build
mkdir build
cd build
cmake .. -DCPPZMQ_BUILD_TESTS=OFF
sudo make -j4 install