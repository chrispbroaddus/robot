#!/bin/bash

# Guarantee that if any command returns non-zero, the script returns immediately
# with a non-zero exit.
set -e

mkdir -p /usr/src/libdc1394 
curl -L https://cytranet.dl.sourceforge.net/project/libdc1394/libdc1394-2/2.2.5/libdc1394-2.2.5.tar.gz | tar xvz -C /usr/src/libdc1394 
cd /usr/src/libdc1394/libdc1394-2.2.5 
./configure --without-x --disable-doxygen-doc --enable-shared --disable-static --disable-sdltest --disable-examples
make -j4 
make install -j4
rm -rf /usr/src/libdc1394/
