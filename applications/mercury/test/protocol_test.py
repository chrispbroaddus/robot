import logging
import re
import signal
import unittest
import unittest.mock
from applications.mercury.lib.protocols import *


class ProcessWatcherProtocolTest(unittest.TestCase):
    @staticmethod
    def create_mock_transport(return_codes, is_closing=[True]):
        transport = unittest.mock.MagicMock()
        transport.get_returncode = unittest.mock.MagicMock(side_effect=return_codes)
        transport.send_signal = unittest.mock.MagicMock()
        transport.kill = unittest.mock.MagicMock()
        transport.close = unittest.mock.MagicMock()
        transport.is_closing = unittest.mock.MagicMock(side_effect=is_closing)
        return transport

    @staticmethod
    def create_mock_recovery_policy():
        policy = unittest.mock.MagicMock()
        policy.notify_process_exited = unittest.mock.MagicMock()
        return policy

    @staticmethod
    def create_mock_node():
        node = unittest.mock.MagicMock()
        node.name = 'Mock Node'
        return node

    def test_events_delivered_by_default(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.process_exited()

        # Make sure that the signal was sent as expected
        recovery.notify_process_exited.assert_called_once_with(node, 0)

    def test_events_inhibited_when_disabled(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.disable_events()
        protocol.process_exited()

        recovery.notify_process_exited.assert_not_called()

    def test_events_delivered_after_reenabling(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.disable_events()
        protocol.enable_events()
        protocol.process_exited()

        recovery.notify_process_exited.assert_called_once_with(node, 0)

    def test_request_stop_not_started(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.request_stop()

        transport.get_returncode.assert_not_called()
        transport.send_signal.assert_not_called()
        transport.kill.assert_not_called()
        transport.close.assert_not_called()

    def test_request_stop_already_exited(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.request_stop()

        transport.get_returncode.assert_called_once_with()
        transport.send_signal.assert_not_called()
        transport.kill.assert_not_called()
        transport.close.assert_not_called()

    def test_request_stop(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([None, 0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.request_stop()

        transport.get_returncode.assert_called_once_with()
        transport.send_signal.assert_called_once_with(signal.SIGTERM)
        transport.kill.assert_not_called()
        transport.close.assert_not_called()

    def test_force_stop_not_started(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.force_stop()

        transport.get_returncode.assert_not_called()
        transport.send_signal.assert_not_called()
        transport.kill.assert_not_called()
        transport.close.assert_not_called()

    def test_force_stop_already_exited(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.force_stop()

        transport.get_returncode.assert_called_once_with()
        transport.send_signal.assert_not_called()
        transport.kill.assert_not_called()
        transport.close.assert_not_called()

    def test_force_stop(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([None, 0], [False])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.force_stop()

        transport.get_returncode.assert_called_once_with()
        transport.send_signal.assert_not_called()
        transport.kill.assert_called_once_with()
        transport.close.assert_called_once_with()

    def test_reset(self):
        node = ProcessWatcherProtocolTest.create_mock_node()
        recovery = ProcessWatcherProtocolTest.create_mock_recovery_policy()
        transport = ProcessWatcherProtocolTest.create_mock_transport([0])
        protocol = ProcessWatcherProtocol(node, recovery)
        protocol.connection_made(transport)
        protocol.disable_events()
        protocol.reset()
        self.assertIsNone(protocol.transport)
        self.assertTrue(protocol.events_enabled)
