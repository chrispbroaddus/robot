#!/usr/bin/env python3
import argparse
import logging

import configuration_checker as cc


def parse_arguments():
    parser = argparse.ArgumentParser(description="configuration-linter")
    parser.add_argument('-c', '--configuration_directory',
                        help='Top level configuration directory to check',
                        required=True)
    parser.add_argument('-v', '--verbose',
                        help='Be verbose',
                        action='store_true',
                        default=False)

    args = parser.parse_args()
    return (args.configuration_directory, args.verbose)


def main():
    (configuration_directory, verbose) = parse_arguments()
    logging.basicConfig(level=logging.DEBUG if verbose else logging.INFO,
                        format='%(asctime)s %(levelname)-8s %(name)-10s: %(message)s')
    logger = logging.getLogger("configuration-linter")
    checker = cc.ConfigurationChecker(logger)
    checker.check(configuration_directory)
    logger.info('Done checking directory [{}]'.format(configuration_directory))


if "__main__" == __name__:
    main()
