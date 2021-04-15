#!/bin/bash

# Guarantee that if any command returns non-zero, the script returns immediately
# with a non-zero exit.
set -e

# Install Tensorflow from source. This guarentees consistency across python and cpp
commit=e9d5ee1ebffba25cef65f1f354b9e4ca9bcea10c
wget https://github.com/tensorflow/tensorflow/archive/$commit.zip
unzip $commit.zip
cd tensorflow-$commit

# Input options for configure newline for enter
python_loc="\n"
python_lib="\n"
jmalloc="y\n"
gc="n\n"
hadoop_fs="n\n"
xla="y\n"
gdr="n\n"
verbs="n\n"
opencl="n\n"
cuda="y\n"
cuda_version="8.0\n"
cuda_loc="/usr/local/cuda\n"
cudnn_version="7\n"
cudnn_loc="usr/lib/x86_64-linux-gnu\n"
compute="6.1\n"
clang_cuda="n\n"
gcc="\n"
mpi="n\n"
opt_flags="\n"

tf_opts="$python_loc$python_lib$jmalloc$gc$hadoop_fs$xla$gdr$verbs$opencl$cuda$cuda_version$cuda_loc$cudnn_version$cudnn_loc$compute$clang_cuda$gcc$mpi$opt_flags"

# Automate configure script with predefinied options (above)
echo -e $tf_opts | ./configure

# Build python and c package
bazel build -c opt --copt=-mavx --copt=-mavx2 --copt=-mfma --copt=-mfpmath=both --copt=-msse4.2 --config=cuda //tensorflow/tools/lib_package:clib
bazel build -c opt --copt=-mavx --copt=-mavx2 --copt=-mfma --copt=-mfpmath=both --copt=-msse4.2 --config=cuda //tensorflow/tools/pip_package:build_pip_package

# Build wheel for python
bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg
# install python package
pip install --upgrade /tmp/tensorflow_pkg/tensorflow-*.whl


# Place c stuff in right location
mkdir -p /usr/local/include/tensorflow/c/
cp tensorflow/c/c_api.h /usr/local/include/tensorflow/c/
chmod +x /usr/local/include/tensorflow/c/c_api.h
cp bazel-bin/tensorflow/libtensorflow.so /usr/local/lib/


# Clean up
cd
rm -rf /tensorflow*

# export locations
echo "LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib" >> /etc/environment
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib" >> /etc/environment
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
