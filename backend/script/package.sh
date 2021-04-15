#!/bin/bash
set -e

# package build
mkdir -p /builds/$BRANCH/$SHA/
cp -r teleop-server /builds/$BRANCH/$SHA/
