import asyncio
import logging
import signal


class ProcessWatcherProtocol(asyncio.SubprocessProtocol):
    """
    Extends SubprocessProcotol for our particular use case. Right now, this is mostly a source of events / information
    flow into a high level recovery policy which will need to react to thinks like program_exited.

    Eventually, there will be a twin of this which implements the progress / liveness side of the watchdog events as a
    complement to this class which can only really provide events regardining the definitive death of components.
    """

    def __init__(self, graph_node, recovery_policy):
        """
        :param graph_node:
        :param recovery_policy:
        """
        self.logger = logging.getLogger('mercury.ProcessWatcherProtocol')
        self.graph_node = graph_node
        self.recovery_policy = recovery_policy
        self.events_enabled = True
        self.transport = None

    def disable_events(self):
        self.events_enabled = False

    def enable_events(self):
        self.events_enabled = True

    def connection_made(self, transport):
        self.logger.info('Process [{}] launched'.format(self.graph_node.name))
        self.transport = transport

    def pipe_data_received(self, fd, data):
        # Nobody cares what output was produced (hopefully this was captured in syslog somewhere by someone else...)
        pass

    def process_exited(self):
        self.logger.info('Process [{}] exited'.format(self.graph_node.name))
        if self.events_enabled and self.recovery_policy is not None:
            return_code = self.transport.get_returncode()
            self.logger.info(
                'Notifying recovery policy that [{}] has exited with return code [{}]'.format(self.graph_node.name,
                                                                                              return_code))
            self.recovery_policy.notify_process_exited(self.graph_node, return_code)

    def request_stop(self):
        self.logger.info('Received request to stop process [{}]'.format(self.graph_node.name))
        # If we have started the process and it is still running...
        if (self.transport is not None) and (self.transport.get_returncode() is None):
            self.logger.info('Sending [{}] SIGTERM to request a graceful stop'.format(self.graph_node.name))
            self.transport.send_signal(signal.SIGTERM)

    def force_stop(self):
        self.logger.info('Received request to force stopping process [{}]'.format(self.graph_node.name))
        if (self.transport is not None) and (self.transport.get_returncode() is None):
            self.logger.info('Forcefully terminating [{}]'.format(self.graph_node.name))
            self.transport.kill()

        self.logger.info('Closing transport for process [{}]'.format(self.graph_node.name))

        if self.transport is not None and not self.transport.is_closing():
            self.transport.close()

    def reset(self):
        self.transport = None
        self.events_enabled = True
