import logging
import re
import unittest
from applications.mercury.lib.process_graph import *
from applications.mercury.proto import managed_process_pb2, watchdog_policy_pb2


class ProcessGraphTest(unittest.TestCase):
    def setUp(self):
        self.logger = logging.getLogger('mercury.ProcessGraphTest')

    def test_add_node_fails_on_missing_name(self):
        """
        Given an otherwise valid ManagedProcess which is missing the name field, call add_node and verify that we
        get the expected exception
        :return:
        """
        mp = managed_process_pb2.ManagedProcess()
        mp.executable_path = '/usr/bin/yes'

        with self.assertRaises(MissingRequiredProtobufField):
            g = ProcessDirectedGraph()
            g.add_node(mp)

    def test_add_node_fails_on_missing_executable_path(self):
        """
        Given an otherwise valid ManagedProcess which is missing the executable_path field, call add_node and verify
        that we get the expected exception.
        :return:
        """
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'

        with self.assertRaises(MissingRequiredProtobufField):
            g = ProcessDirectedGraph()
            g.add_node(mp)

    def test_warn_on_missing_policy(self):
        """
        Given a valid ManagedProcess, verify that add_node succeeds and that the fields in the ManagedProcess are
        correctly transcribed over. Expect a warning that the watchdog policy is missing
        :return:
        """
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        g = ProcessDirectedGraph()

        with self.assertWarns(MissingPolicyWarning):
            n = g.add_node(mp)
            self.assertTrue(n.failure_is_catastrophic)
            self.assertFalse(n.ignore_failures)
            self.assertFalse(n.propagate_to_descendants)
            self.assertEqual(n.initial_watchdog_time_seconds, 1)
            self.assertEqual(n.watchdog_period_seconds, 0.01)

    def test_add_node_with_ignore_failures_policy(self):
        """
        Given a valid ManagedProcess, verify that add_node succeeds and that the fields in the ManagedProcess are
        correctly transcribed over.
        :return:
        """
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        mp.ignore_policy.dummy = 0
        g = ProcessDirectedGraph()
        n = g.add_node(mp)
        self.assertFalse(n.failure_is_catastrophic)
        self.assertTrue(n.ignore_failures)
        self.assertFalse(n.propagate_to_descendants)

    def test_add_node_with_catastrophic_failure_policy(self):
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        mp.fail_policy.liveness_check_period_milliseconds = 1000
        mp.fail_policy.first_liveness_check_seconds = 1
        g = ProcessDirectedGraph()
        n = g.add_node(mp)
        self.assertTrue(n.failure_is_catastrophic)
        self.assertFalse(n.ignore_failures)
        self.assertFalse(n.propagate_to_descendants)
        self.assertEqual(n.initial_watchdog_time_seconds, 1)
        self.assertEqual(n.watchdog_period_seconds, 1.0)

    def test_add_node_with_relaunch_policy_ignore_descendants(self):
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        mp.relaunch_policy.liveness_check_period_milliseconds = 1000
        mp.relaunch_policy.first_liveness_check_seconds = 1
        mp.relaunch_policy.propagate_to_descendants = False
        g = ProcessDirectedGraph()
        n = g.add_node(mp)
        self.assertFalse(n.failure_is_catastrophic)
        self.assertFalse(n.ignore_failures)
        self.assertFalse(n.propagate_to_descendants)
        self.assertEqual(n.initial_watchdog_time_seconds, 1)
        self.assertEqual(n.watchdog_period_seconds, 1.0)

    def test_add_node_with_relaunch_policy_include_descendants(self):
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        mp.relaunch_policy.liveness_check_period_milliseconds = 1000
        mp.relaunch_policy.first_liveness_check_seconds = 1
        mp.relaunch_policy.propagate_to_descendants = True
        g = ProcessDirectedGraph()
        n = g.add_node(mp)
        self.assertFalse(n.failure_is_catastrophic)
        self.assertFalse(n.ignore_failures)
        self.assertTrue(n.propagate_to_descendants)
        self.assertEqual(n.initial_watchdog_time_seconds, 1)
        self.assertEqual(n.watchdog_period_seconds, 1.0)

    def test_add_node_fails_on_duplicated_name(self):
        """
        Given two (possibly identical) ManagedProcess instances which have the same name, verify that the first call to
        add_node succeeds and the second call fails with the expected exceptions
        :return:
        """
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        mp.ignore_policy.dummy = 0
        g = ProcessDirectedGraph()
        g.add_node(mp)

        with self.assertRaises(DuplicateManagedProcessName):
            g.add_node(mp)

    def test_build_fails_on_missing_publisher(self):
        """
        Add a single ManagedProcess instance which requires a topic, then call build and verify that we get the expected
        exception.
        :return:
        """
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        mp.required_topics.append('dummy')
        mp.ignore_policy.dummy = 0

        g = ProcessDirectedGraph()
        g.add_node(mp)

        with self.assertRaises(GraphBuildError):
            g.build()

    def test_build_fails_on_missing_dependency(self):
        """
        Add a single ManagedProcess instance which has an additional_dependency, then call build and verify that we get
        the expected exception
        :return:
        """
        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes'
        mp.executable_path = '/usr/bin/yes'
        mp.additional_process_dependencies.append('dummy')
        mp.ignore_policy.dummy = 0

        g = ProcessDirectedGraph()
        g.add_node(mp)

        with self.assertRaises(GraphBuildError):
            g.build()

    def test_has_cycle_with_cycle(self):
        """
        Create a simple ring structure, call build, then call has_cycle -- should return true
        :return:
        """
        g = ProcessDirectedGraph()

        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes1'
        mp.executable_path = '/usr/bin/yes'
        mp.additional_process_dependencies.append('yes2')
        mp.ignore_policy.dummy = 0

        g.add_node(mp)

        mp.Clear()
        mp.process_name = 'yes2'
        mp.executable_path = '/usr/bin/yes'
        mp.additional_process_dependencies.append('yes1')
        mp.ignore_policy.dummy = 0

        g.add_node(mp)
        g.build()
        self.assertTrue(g.has_cycle())

    def test_cycle_without_cycle(self):
        """
        Create a simple linear structure, call build, then call has_cycle -- should return false
        :return:
        """
        g = ProcessDirectedGraph()

        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes1'
        mp.executable_path = '/usr/bin/yes'
        mp.additional_process_dependencies.append('yes2')
        mp.ignore_policy.dummy = 0

        g.add_node(mp)

        mp.Clear()
        mp.process_name = 'yes2'
        mp.executable_path = '/usr/bin/yes'
        mp.ignore_policy.dummy = 0

        g.add_node(mp)
        g.build()
        self.assertFalse(g.has_cycle())

    def test_topological_sort_throws_on_cycle(self):
        """
        Build a simple ring structure, call build, then call topological sort and see that we get the expected exception
        :return:
        """
        g = ProcessDirectedGraph()

        mp = managed_process_pb2.ManagedProcess()
        mp.process_name = 'yes1'
        mp.executable_path = '/usr/bin/yes'
        mp.additional_process_dependencies.append('yes2')
        mp.ignore_policy.dummy = 0

        g.add_node(mp)

        mp.Clear()
        mp.process_name = 'yes2'
        mp.executable_path = '/usr/bin/yes'
        mp.additional_process_dependencies.append('yes1')
        mp.ignore_policy.dummy = 0

        g.add_node(mp)
        g.build()

        with self.assertRaises(GraphIsCyclicError):
            g.topological_sort()

    def test_topological_sort_happy_path(self):
        """
        Build a lattice / multipartite graph such that the each node in partition i has edges to every node in partition
        i + 1; perform topological sort, verify that all nodes in partition i are processed before nodes in partition
        i + 1
        :return:
        """
        partitions = 5
        nodes_per_partition = 4

        g = ProcessDirectedGraph()

        for p in range(0, partitions):
            for n in range(0, nodes_per_partition):
                mp = managed_process_pb2.ManagedProcess()
                mp.process_name = 'partition_{}_node_{}'.format(p, n)
                mp.executable_path = '/usr/bin/yes'
                mp.ignore_policy.dummy = 0

                if p > 0:
                    for n2 in range(0, nodes_per_partition):
                        mp.required_topics.append('topic_partition_{}_node_{}'.format(p - 1, n2))

                mp.provided_topics['topic_partition_{}_node_{}'.format(p, n)] = 0

                g.add_node(mp)

        g.build()

        order = g.topological_sort()
        matcher = re.compile(r'^partition_(\d+)')
        partitions = []

        for node in order:
            m = matcher.match(node.name)
            self.assertIsNotNone(m)
            partitions.append(int(m.group(1)))

        prev = -1
        for p in partitions:
            self.assertLessEqual(prev, p)
            prev = p

        def test_breadth_first_search_happy_path(self):
            """
            Build a lattice / multipartite graph such that the each node in partition i has edges to every node in partition
            i + 1; perform bfs, verify that there is only one node in parition 0 and all nodes in partition i are processed
            before nodes in partition i + 1
            :return:
            """
            partitions = 5
            nodes_per_partition = 10

            g = ProcessDirectedGraph()

            for p in range(0, partitions):
                for n in range(0, nodes_per_partition):
                    mp = managed_process_pb2.ManagedProcess()
                    mp.process_name = 'partition_{}_node_{}'.format(p, n)
                    mp.executable_path = '/usr/bin/yes'
                    mp.ignore_policy.dummy = 0

                    if p > 0:
                        for n2 in range(0, nodes_per_partition):
                            mp.required_topics.append('topic_partition_{}_node_{}'.format(p - 1, n2))

                    mp.provided_topics['topic_partition_{}_node_{}'.format(p, n)] = 0

                    g.add_node(mp)

            g.build()

            matcher = re.compile(r'^partition_(\d+)')
            partitions = []

            def visit(node, direction):

                if 'enter' == direction:
                    m = matcher.match(node.name)
                    self.assertIsNotNone(m)
                    partitions.append(int(m.group(1)))

            g.breadth_first_traversal(visit, None)

            zeroCount = 0
            prev = -1
            for p in partitions:
                self.assertLessEqual(prev, p)
                prev = p

                if 0 == p:
                    zeroCount += 1

            self.assertEqual(zeroCount, 1)
