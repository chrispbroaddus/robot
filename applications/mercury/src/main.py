import argparse
import asyncio
import functools
import google.protobuf.text_format as tf
import logging
import re
import signal
import sys
from applications.mercury.lib.process_graph import *
from applications.mercury.lib.protocols import *
from applications.mercury.lib.recovery_policy import *
from applications.mercury.lib.system_uuid import *
from applications.mercury.proto import system_description_pb2



def parse_arguments():
    parser = argparse.ArgumentParser(description="mercury")
    parser.add_argument('-s', '--system',
                        help='System description (in pbtext format) to run',
                        required=True)
    parser.add_argument('-v', '--verbose',
                        help='Be verbose',
                        action='store_true',
                        default=False)

    args = parser.parse_args()
    return (args.system, args.verbose)


def load_process_graph(path, system_uuid):
    logger = logging.getLogger('mercury')
    g = ProcessDirectedGraph()

    substituted_path = substitute_system_uuid(path, system_uuid)
    logger.info('Loading system description from [{}] --> [{}]'.format(path, substituted_path))
    system_definition = system_description_pb2.SystemDescription()
    with open(str(path), 'rt') as f:
        tf.Merge(f.read(), system_definition)

    sys = substitute_system_uuid_in_system_description(system_definition, system_uuid)

    logger.info('Populating process graph nodes')
    for p in sys.processes:
        g.add_node(p)

    logger.info('Building edge lists')
    g.build()
    return g


def handle_shutdown_signal(loop, graph):
    logger = logging.getLogger('mercury')
    logger.info('Shutdown requested')
    loop.stop()


def probe_system_uuid():
    path = '/zippy-persistent/ZIPPY-SERIAL-NUMBER'
    with open(path, 'rt') as f:
        return f.readline().strip().upper()


def main():
    logging.basicConfig(level=logging.INFO, format='%(asctime)s %(levelname)-8s %(name)-30s: %(message)s')
    logger = logging.getLogger('mercury')
    loop = asyncio.get_event_loop()

    (path, verbose) = parse_arguments()
    if verbose:
        logger.root.setLevel(logging.DEBUG)
        loop.set_debug(True)

    system_uuid = probe_system_uuid()

    logger.info('Loading process graph for system_uuid=[{}]'.format(system_uuid))
    g = load_process_graph(path, system_uuid)

    logger.info('Installing signal handlers')
    loop.add_signal_handler(signal.SIGINT, functools.partial(handle_shutdown_signal, loop, g))
    loop.add_signal_handler(signal.SIGTERM, functools.partial(handle_shutdown_signal, loop, g))

    logger.info('Instantiating recovery policy')
    recovery = RecoveryPolicy(g, loop)

    logger.info('Scheduling launch')
    asyncio.ensure_future(recovery.launch_processes(), loop=loop)

    logger.info('Running event loop')
    try:
        loop.run_forever()
    finally:
        loop.close()


if '__main__' == __name__:
    main()
