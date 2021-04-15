#!/bin/bash

# Guarantee that if any command returns non-zero, the script returns immediately
# with a non-zero exit.
set -e

mkdir -p /usr/src/opencv
curl -L https://codeload.github.com/opencv/opencv/tar.gz/3.2.0 | tar xvz -C /usr/src/opencv
mkdir -p /usr/src/opencv/opencv-3.2.0/build
cd /usr/src/opencv/opencv-3.2.0/build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j4
make install -j4
rm -rf /usr/src/opencv
