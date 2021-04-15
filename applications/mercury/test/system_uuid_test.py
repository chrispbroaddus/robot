import logging
import re
import signal
import unittest
import unittest.mock
from applications.mercury.lib.system_uuid import *
from applications.mercury.proto import managed_process_pb2, system_description_pb2

class SystemUUIDTest(unittest.TestCase):
    null_uuid='00000000-0000-0000-0000-000000000000'

    def test_simple_substitution_no_match(self):
        input='O Fortuna velut luna statu variabilis, semper crescis aut decresis.'
        expected=input
        output=substitute_system_uuid(input, SystemUUIDTest.null_uuid)
        self.assertEqual(output, expected)

    def test_simple_substitution_single_match(self):
        input='Hello from ZIPPY-SERIAL-NUMBER'
        expected='Hello from ' + SystemUUIDTest.null_uuid
        output=substitute_system_uuid(input, SystemUUIDTest.null_uuid)
        self.assertEqual(output, expected)

    def test_simple_substitution_multiple_matches(self):
        input='Hello from ZIPPY-SERIAL-NUMBERZIPPY-SERIAL-NUMBER/ZIPPY-SERIAL-NUMBER-'
        expected='Hello from ' + SystemUUIDTest.null_uuid + SystemUUIDTest.null_uuid + '/' + SystemUUIDTest.null_uuid + '-'
        output=substitute_system_uuid(input, SystemUUIDTest.null_uuid)
        self.assertEqual(output, expected)

    def test_substitute_executable_path(self):
        dummy = system_description_pb2.SystemDescription()

        mp = dummy.processes.add()
        mp.process_name = 'dummy'
        mp.executable_path = '/opt/zippy/bin/magic.ZIPPY-SERIAL-NUMBER'

        expected = '/opt/zippy/bin/magic.' + SystemUUIDTest.null_uuid
        substitute_system_uuid_in_system_description(dummy, SystemUUIDTest.null_uuid)
        self.assertEqual(dummy.processes[0].executable_path, expected)

    def test_substitute_arguments(self):
        dummy = system_description_pb2.SystemDescription()

        mp = dummy.processes.add()
        mp.process_name = 'dummy'
        mp.executable_path = '/opt/zippy/bin/magic'
        mp.arguments.append('--serial=ZIPPY-SERIAL-NUMBER')
        expected_arg = '--serial=' + SystemUUIDTest.null_uuid

        substitute_system_uuid_in_system_description(dummy, SystemUUIDTest.null_uuid)
        self.assertEqual(dummy.processes[0].arguments[0], expected_arg)

    def test_substitute_environment_key(self):
        dummy = system_description_pb2.SystemDescription()

        mp = dummy.processes.add()
        mp.process_name = 'dummy'
        mp.executable_path = '/opt/zippy/bin/magic'
        mp.arguments.append('--serial=ZIPPY-SERIAL-NUMBER')
        expected_arg = '--serial=' + SystemUUIDTest.null_uuid

        mp.environment['ZIPPY-SERIAL-NUMBER'] = '0'
        expected_key = SystemUUIDTest.null_uuid
        expected_value = '0'

        substitute_system_uuid_in_system_description(dummy, SystemUUIDTest.null_uuid)
        self.assertEqual(dummy.processes[0].arguments[0], expected_arg)
        self.assertIn(expected_key, dummy.processes[0].environment)
        self.assertEqual(expected_value, dummy.processes[0].environment[expected_key])

    def test_substitute_environment_value(self):
        dummy = system_description_pb2.SystemDescription()

        mp = dummy.processes.add()
        mp.process_name = 'dummy'
        mp.executable_path = '/opt/zippy/bin/magic'
        mp.arguments.append('--serial=ZIPPY-SERIAL-NUMBER')
        expected_arg = '--serial=' + SystemUUIDTest.null_uuid

        mp.environment['SERIAL'] = 'ZIPPY-SERIAL-NUMBER'
        expected_key = 'SERIAL'
        expected_value = SystemUUIDTest.null_uuid

        substitute_system_uuid_in_system_description(dummy, SystemUUIDTest.null_uuid)
        self.assertEqual(dummy.processes[0].arguments[0], expected_arg)
        self.assertIn(expected_key, dummy.processes[0].environment)
        self.assertEqual(expected_value, dummy.processes[0].environment[expected_key])
