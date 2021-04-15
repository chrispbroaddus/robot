#!/bin/bash

# Guarantee that if any command returns non-zero, the script returns immediately
# with a non-zero exit.
set -e

curl -O https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/flycapture2-2.11.3.121-amd64-pkg.tgz
sha256sum flycapture2-2.11.3.121-amd64-pkg.tgz  | awk '{if ($1 != "0a6f1a856126da6e9d73940e8fce77839d47af68f179cebcefa178a8c8ad651f") {exit -1}}'
tar -xvf flycapture2-2.11.3.121-amd64-pkg.tgz
cd flycapture2-2.11.3.121-amd64/
dpkg -i libflycapture-2*
cd ..
rm flycapture2-2.11.3.121-amd64-pkg.tgz
rm -rf flycapture2-2.11.3.121-amd64
