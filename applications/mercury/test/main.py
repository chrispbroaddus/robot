import logging
import unittest
from process_graph_test import ProcessGraphTest
from protocol_test import ProcessWatcherProtocolTest
from recovery_policy_test import StateMachineTransitionTest
from streamingstatistics_test import StreamingStatisticsTest
from system_uuid_test import SystemUUIDTest

def load_tests(loader, standard_tests, pattern):
    test_cases = (ProcessGraphTest,
                  StreamingStatisticsTest,
                  ProcessWatcherProtocolTest,
                  StateMachineTransitionTest,
                  SystemUUIDTest)

    suite = unittest.TestSuite()
    for test_class in test_cases:
        suite.addTests(loader.loadTestsFromTestCase(test_class))

    return suite


if "__main__" == __name__:
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)-8s %(name)-10s: %(message)s')
    unittest.main()
