#!/bin/bash

# Guarantee that if any command returns non-zero, the script returns immediately
# with a non-zero exit.
set -e

pip --no-cache-dir install --ignore-installed \
    awscli

