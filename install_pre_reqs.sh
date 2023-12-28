!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
cd "$parent_path"

sudo apt update

installpackage () {
  REQUIRED_PKG=$1
  PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG|grep "install ok installed")
  echo "=== Checking for $REQUIRED_PKG"
  if [ "" = "$PKG_OK" ]; then
    echo "=== Installing $REQUIRED_PKG."
    sudo apt-get -y install $REQUIRED_PKG
  else
    echo "=== Package already installed."
  fi
}

installpackage "gpiod"
installpackage "libgpiod-dev"

git submodule update --init --recursive

# cd "$parent_path"/src/libraries/libzmq
# mkdir build
# cd build
# cmake .. -DCPPZMQ_BUILD_TESTS=OFF
# sudo make -j4 install

# cd "$parent_path"/src/libraries/cppzmq
# rm -fr build
# mkdir build
# cd build
# cmake .. -DCPPZMQ_BUILD_TESTS=OFF
# sudo make -j4 install