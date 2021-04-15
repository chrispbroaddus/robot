#!/usr/bin/env python
import argparse
import os
import logging
import subprocess
import sys

logging.basicConfig(level=logging.INFO)

KCOV_BINARY = 'kcov'
KCOV_OUTPUT_DIRECTORY = 'bazel-kcov'
BAZEL_BINARY_DIRECTORY = 'bazel-bin'
SCRIPT_RELATIVE_DIRECTORY =  '../../../'

def coverage(test, arguments):
    script_directory = os.path.realpath(__file__)
    root_directory = os.path.abspath(os.path.join(script_directory, SCRIPT_RELATIVE_DIRECTORY))
    kcov_directory = os.path.join(root_directory, KCOV_OUTPUT_DIRECTORY)
    cmd = [KCOV_BINARY,
            kcov_directory,
            os.path.join(root_directory,
                         BAZEL_BINARY_DIRECTORY,
                         test['path'])]
    cmd.extend(arguments)
    process = subprocess.Popen(cmd,
                               cwd=root_directory,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)

    stdout, stderr = process.communicate()
    logging.info('stdout')
    logging.info(stdout)
    logging.info('stderr:')
    logging.info(stderr)
    return process.returncode

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('path')
    test, arguments = parser.parse_known_args()
    sys.exit(coverage(vars(test), arguments))

if __name__ == "__main__":
    main()
