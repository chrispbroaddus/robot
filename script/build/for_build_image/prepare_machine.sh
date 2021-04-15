#!/bin/bash

set -e
ME=`readlink ${BASH_SOURCE[0]} || echo ${BASH_SOURCE[0]}`
DIR=`dirname $ME`
echo $DIR

${DIR}/install_apt_packages.sh
${DIR}/install_aws_cli.sh
${DIR}/install_dc1394.sh
${DIR}/install_flycapture.sh
${DIR}/install_opencv32.sh
${DIR}/install_buildifier.sh
${DIR}/install_zeromq421.sh
# Tensorflow installed separately

