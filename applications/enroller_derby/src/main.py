#!/usr/bin/env python3

import argparse
import base64
import http.client
import json
import logging
import os
import os.path
import re
import ssl
import stat
import uuid
from http import HTTPStatus

# Global logger because I'm lazy
logger = logging.getLogger("enroller-derby")

# Base path on the zippy host-side. Treat this as a const.
base_path = '/var/zippy'


def parse_arguments():
    parser = argparse.ArgumentParser(description="")
    parser.add_argument('-v', '--verbose',
                        help='Be verbose',
                        action='store_true',
                        default=False)

    args = parser.parse_args()
    return (args.verbose,)


def ensure_directory_structure_exists():
    logger.info('Ensuring that [{}] exists'.format(base_path))
    os.makedirs(os.fsencode(base_path), exist_ok=True)

    logger.info('Ensuring permissions are u=rwX,g=rX,o=rX for [{}]'.format(base_path))
    user_perms = stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR
    group_perms = stat.S_IRGRP | stat.S_IXGRP
    other_perms = stat.S_IROTH | stat.S_IXOTH
    os.chmod(os.fsencode(base_path), user_perms | group_perms | other_perms)


def ensure_uuid_file():
    path = '{}/ZIPPY-SERIAL-NUMBER'.format(base_path)
    if not os.path.exists(path):
        token_size_bits = 1024
        bits_per_byte = 8
        random_token = None

        with open('/dev/random', 'rb') as file:
            random_token = base64.b64encode(file.read(token_size_bits // bits_per_byte)).decode()

        domain_uuid = uuid.uuid5(uuid.NAMESPACE_DNS, "teleop.zippy.ai")

        id = uuid.uuid5(domain_uuid, random_token)
        logger.info(
            'Detected that this vehicle has not yet been assigned a serial number. This is now [{}]'.format(str(id).upper()))

        # Write this to disk
        with open(path, 'wt') as f:
            print(str(id).upper(), file=f)

        # Ensure sane permissions
        logger.info('Ensuring correct permissions for [{}]'.format(path))
        os.chmod(os.fsencode(path), stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH)

        system_uuid = ''
        with open(path, 'rt') as f:
            system_uuid = f.readline().strip().upper()

        if re.match('[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}', system_uuid) is None:
            raise RuntimeError('Somehow your UUID file [{}] contains an invalid GUID [{}]'.format(path, system_uuid))

        return system_uuid
    else:
        raise RuntimeError('Could not find expected file [{}]'.format(path))


def ensure_authentication_token_file(system_uuid):
    path = '{}/ZIPPY-CREDENTIALS.jwt'.format(base_path)

    token_source = {'host': 'mission-control.zippy.ai',
                    'timeout': 15,
                    'context': ssl.create_default_context(),
                    'headers': {},
                    'url': '/api/v1/vehicle/token/{}'.format(system_uuid),
                    'method': 'POST'}

    if not os.path.exists(path):
        token = ''
        logger.info('Detected that this vehicle does not yet have an authentication token.')

        logger.info(
            'Creating HTTP connection to [{}]'.format(token_source['host']))
        connection = http.client.HTTPConnection(token_source['host'])

        try:
            logger.info('Using method [{}] to request URL [{}]'.format(token_source['method'], token_source['url']))
            connection.request(token_source['method'], token_source['url'], headers=token_source['headers'])

            with connection.getresponse() as response:
                logger.info('Response status = [{}], reason = [{}]'.format(response.status, response.reason))
                if response.status < 200 or response.status >= 300:
                    raise RuntimeError('Failed to request token (status should be [{}], got [{}] instead)'.format(
                        HTTPStatus.CREATED, response.status))

                raw = response.read()
                parsed = json.loads(raw.decode())
                token = parsed['token']
                logger.info('Received token')
        finally:
            connection.close()

        # Write this to disk
        with open(path, 'wt') as f:
            print(token, file=f)

        # Ensure sane permissions
        logger.info('Ensuring correct permissions for [{}]'.format(path))
        os.chmod(os.fsencode(path), stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH)


def main():
    (verbose,) = parse_arguments()

    logging.basicConfig(level=logging.INFO, format='%(asctime)s %(levelname)-8s %(name)-10s: %(message)s')

    if verbose:
        logging.root.setLevel(logging.DEBUG)

    logger.info('Ensuring that the expected directory structure exists')

    if 0 != os.geteuid():
        raise PermissionError('This script needs to be run as root.')

    ensure_directory_structure_exists()
    system_uuid = ensure_uuid_file()
    ensure_authentication_token_file(system_uuid)


if "__main__" == __name__:
    main()
