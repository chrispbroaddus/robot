#!/bin/bash

set -euo pipefail

if [[ $EUID -ne 0 ]]; then
  echo "This script should be run using sudo"
  exit 1
fi

temporary=$(mktemp -d)

if [ ! -d $temporary ]; then
  echo "Failed to create temporary directory!"
  exit 1
fi

buildifier_compressed=$temporary/buildifier.tar.gz
curl -o $buildifier_compressed https://s3-us-west-2.amazonaws.com/zippy.build/dependencies/38ca420f1bb19e9737f111ea390e923290516bc8bf3e56ce3c71116cc2147a49.tar.gz

if [ ! -e $buildifier_compressed ]; then
  echo "Failed to fetch buildifier!"
  exit 1
fi

tar xzvf $buildifier_compressed -C $temporary
if [ $? -ne 0 ]; then
  echo "Failed to extract buildifier!"
  exit 1
fi

buildifier_install=$temporary/buildifier_0.1.1_amd64.deb

if [ ! -e $buildifier_install ]; then
  echo ".deb does not exist, something went wrong!"
  exit 1
fi

dpkg -i $buildifier_install
rm $temporary/buildifier_0.1.1_amd64.deb
