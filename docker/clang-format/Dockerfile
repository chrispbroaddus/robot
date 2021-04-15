FROM ubuntu:16.04

RUN apt-get update && apt-get install -y clang-format-3.8 git

RUN mkdir /zippy
WORKDIR /zippy

ENTRYPOINT ["/bin/bash"]
