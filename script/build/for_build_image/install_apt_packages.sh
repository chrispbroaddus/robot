#!/bin/bash

# Guarantee that if any statement fails, the script bails immediate. 
set -e

apt-get update
apt-get install -y curl software-properties-common openjdk-8-jdk wget unzip

wget -q https://github.com/bazelbuild/bazel/releases/download/0.5.4/bazel-0.5.4-installer-linux-x86_64.sh
chmod +x bazel-0.5.4-installer-linux-x86_64.sh
./bazel-0.5.4-installer-linux-x86_64.sh
rm bazel-0.5.4-installer-linux-x86_64.sh

# Install the PPA for GCC 4.9 which is required for C++14
add-apt-repository -y ppa:ubuntu-toolchain-r/test
apt-get update -o Dir::Etc::sourcelist="sources.list.d/ubuntu-toolchain-r-ubuntu-test-xenial.list" -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0"

# Install all the remaining apt packages we need
# Note that nodejs-legacy is required to have "node" resolve to the nodejs
# binary.
apt-get install -y astyle
apt-get install -y autoconf
apt-get install -y automake
apt-get install -y build-essential
apt-get install -y clang
apt-get install -y clang-format
apt-get install -y cmake
apt-get install -y git
apt-get install -y kcov
apt-get install -y libavcodec-dev
apt-get install -y libavformat-dev
apt-get install -y libboost-all-dev=1.58.0.1ubuntu1
apt-get install -y libczmq-dev
apt-get install -y libfreetype6-dev
apt-get install -y libgl1-mesa-dev
apt-get install -y libglm-dev
apt-get install -y libglew-dev
apt-get install -y libglu1-mesa-dev
apt-get install -y libgps-dev
apt-get install -y libglm-dev
apt-get install -y libglfw3-dev
apt-get install -y libgtk2.0-dev
apt-get install -y libpng12-dev 
apt-get install -y libpython3.5
apt-get install -y libraw1394-11
apt-get install -y libraw1394-dev
apt-get install -y libssl-dev
apt-get install -y libswscale-dev
apt-get install -y libtool
apt-get install -y libunwind-dev
apt-get install -y libusb-dev
apt-get install -y libusb-1.0-0-dev
apt-get install -y libusb-1.0-doc
apt-get install -y libx11-dev
apt-get install -y libxext-dev
apt-get install -y make
apt-get install -y mesa-common-dev
apt-get install -y pkg-config
apt-get install -y python
apt-get install -y python-dev
apt-get install -y python-pathlib
apt-get install -y python-numpy
apt-get install -y python-wheel
apt-get install -y python-pip
apt-get install -y python3
apt-get install -y rsync 
apt-get install -y unzip
