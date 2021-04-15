[![CircleCI](https://circle.zippy.ai/gh/zippyai/zippy/tree/master.svg?style=svg&circle-token=2370db92a43e1aa0688ddb1fa582af7265f24944)](https://circle.zippy.ai/gh/zippyai/zippy/tree/master)

# Setup

## Install IDE

Known good versions:

* CLion Version 2016.3
* CLion Bazel Plug-in Version 2017.04.03

## Clone the repo

```
git clone git@github.com:zippyai/zippy.git
cd zippy
./script/git/install-hooks         # sets up clang-format pre-commit hooks
```

**Important:** You must run the `install-hooks` script above in order to have
your code formatted before commits. Without this, you will get CI failures
related to formatting. You will need to install clang-format locally as
described below.

## Install dependencies (macOS)

```
brew install bazel
brew install protobuf
brew install clang-format@3.8
```
### Install sdl2 (If data_logger package is required)
```
brew install sdl2
```

## Install dependencies (Ubuntu)

1. Before you clone the repo, download and run (as root) [script/build/for_build_image/install_git_lfs.sh](script/build/for_build_image/install_git_lfs.sh). If you have already cloned, you will want to delete your old clone and re-clone once you have git lfs setup.

2. Once you have a clean clone with git lfs support, you can bring your developer box up-to-date, run [script/build/for_build_image/prepare_machine.sh](script/build/for_build_image/prepare_machine.sh)

# Building with bazel

Build one package:
```
bazel build packages/$PACKAGE_NAME
```

Build all packages:
```
bazel build packages/*
```

Bazel has a powerful
[query language](https://bazel.build/versions/master/docs/query.html). For
example, to query and build all of the protobuf targets, invoke:

```
bazel build $(bazel query 'filter(".*proto*", kind("cc_library* rule",
//packages/...))')
```

# Continuous integration

Every commit created on this repository will be automatically built and tested by CircleCI Enterprise hosted in our AWS. You must get a green build on CircleCI before merging a pull request.

CircleCI builds are defined in the circle.yml file at the root of this repository. To add dependencies needed by your code, modify the circle.yml file and include the changes with your pull request.

## Running the reformat scripts manually

Zippy uses clang-format to format all code using a standard layout policy. If you installed the git commit hooks as described above then clang-format will run automatically on each commit. To manually reformat your code, run:
```
cd zippy
./script/dockerized-fix-formatting
```

This will pull the docker image `zippyai/clang-format`, which contains the correct version of clang-format used throughout CI.

Alternatively, you can install clang-format 4.0 locally and run:
```
cd zippy
./script/fix-formatting
```

## Debugging CI

To diagnose CI errors, you can re-build from a commit and ssh into the exact build environment.

- Find the build you want to diagnose in the [dashboard](https://circle.zippy.ai/gh/zippyai/zippy)
- In the upper right hand corner there is a rebuild button. Click the dropdown arrow to the right of it.
- Choose rebuild with ssh
- The new build will show up in the dashboard. Click the new build.
- Click the "Debug via SSH" tab in the middle of the screen.
- There will be a command you can copy/paste to ssh to the build environment
  - If done enough, you may have to remove a line in your `~/.ssh/known_hosts` or add `StrictHostKeyChecking no` in your ssh config
