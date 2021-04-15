#!/bin/bash

# Guarantee that if any command returns non-zero, the script returns immediately
# with a non-zero exit.
set -e

curl -O https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/zeromq_4.2.1-1_amd64.deb
sha256sum zeromq_4.2.1-1_amd64.deb  | awk '{if ($1 != "894d3ccecbd88e0dbb17e647637dad93130be0676074a9616ed0b78b30157580") {exit -1}}'
dpkg -i zeromq_4.2.1-1_amd64.deb
rm zeromq_4.2.1-1_amd64.deb
