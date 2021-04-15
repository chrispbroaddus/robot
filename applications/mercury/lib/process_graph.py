import collections
import io
import logging
import os
import pathlib
import sys
import warnings
from enum import Enum


class NodeColor(Enum):
    WHITE = 1
    GRAY = 2
    BLACK = 3


class DuplicateManagedProcessName(RuntimeError):
    """
    Thrown when we detect the same name being used by more than one managed process.
    """
    pass


class MissingRequiredProtobufField(RuntimeError):
    """
    Thrown when we detect a required protobuf field is missing
    """
    pass


class GraphBuildError(RuntimeError):
    """
    Thrown when something goes wrong when building our graph adjacency information.
    """
    pass


class GraphIsCyclicError(RuntimeError):
    """
    Thrown by operations that require acyclic graphs (e.g. topological sort) if they detect a cycle
    """
    pass


class UnknownWatchdogPolicy(RuntimeError):
    """
    Thrown if we see a watchdog policy which we don't understand / know about
    """
    pass


class MissingPolicyWarning(Warning):
    """
    Raised if a process is missing a watchdog policy.
    """
    pass


class ProcessNode(object):
    @staticmethod
    def _get_policy(managed_process_pb):
        initial_watchdog_time_seconds = 1
        watchdog_period_seconds = 0.01
        failure_is_catastrophic = True
        ignore_failure = False
        propagate_to_descendants = False

        chosen_policy = managed_process_pb.WhichOneof('policy')
        if None == chosen_policy:
            fmt = 'Did not specify any policy for process [{}]. Defaulting to catastrophic failure policy.'
            msg = fmt.format(managed_process_pb.process_name)
            warnings.warn(MissingPolicyWarning(msg))
        elif 'fail_policy' == chosen_policy:
            initial_watchdog_time_seconds = managed_process_pb.fail_policy.first_liveness_check_seconds
            watchdog_period_seconds = managed_process_pb.fail_policy.liveness_check_period_milliseconds / 1000.0
        elif 'relaunch_policy' == chosen_policy:
            failure_is_catastrophic = False
            initial_watchdog_time_seconds = managed_process_pb.relaunch_policy.first_liveness_check_seconds
            watchdog_period_seconds = managed_process_pb.relaunch_policy.liveness_check_period_milliseconds / 1000.0
            propagate_to_descendants = managed_process_pb.relaunch_policy.propagate_to_descendants
        elif 'ignore_policy' == chosen_policy:
            failure_is_catastrophic = False
            ignore_failure = True
        else:
            raise UnknownWatchdogPolicy(
                'Don\'t know what to do about watchdog policy with name [{}] for process [{}]'.format(chosen_policy,
                                                                                                      managed_process_pb.process_name))

        return (initial_watchdog_time_seconds, watchdog_period_seconds, failure_is_catastrophic, ignore_failure,
                propagate_to_descendants)

    """
    Pretty much all the state associated with a particular managed process.
    """

    def __init__(self, managed_process_pb):
        super(ProcessNode, self).__init__()

        # Book-keeping for process management (mostly torn directly out of the PB)
        if managed_process_pb.process_name:
            self.name = managed_process_pb.process_name
        else:
            raise MissingRequiredProtobufField('Expect field "process_name" to be populated in protobuf')

        if managed_process_pb.executable_path:
            self.executable_path = managed_process_pb.executable_path
        else:
            raise MissingRequiredProtobufField('Expect field "executable_path" to be populated in protobuf')

        p = pathlib.Path(self.executable_path)
        if not p.exists() or not p.is_file():
            raise FileNotFoundError(
                'For process [{}], specified executable path [{}] either does not exist or is not a file'.format(
                    self.name, self.executable_path))

        if not os.access(str(p.resolve()), os.R_OK | os.X_OK):
            raise PermissionsError(
                'For process [{}], specified executable path [{}] does not appear to be executable or readable'.format(
                    self.name, self.executable_path))

        (self.initial_watchdog_time_seconds, self.watchdog_period_seconds, self.failure_is_catastrophic,
         self.ignore_failures, self.propagate_to_descendants) = ProcessNode._get_policy(managed_process_pb)

        self.arguments = managed_process_pb.arguments
        self.environment = managed_process_pb.environment
        self.required_hardware = managed_process_pb.required_hardware
        self.provided_topics = managed_process_pb.provided_topics
        self.required_topics = managed_process_pb.required_topics
        self.additional_dependencies = managed_process_pb.additional_process_dependencies
        if managed_process_pb.logdir:
            self.logfile = managed_process_pb.logdir + '/' + managed_process_pb.process_name + '.log'
        else:
            self.logfile = None

        # Process exit / launch monitoring
        self.process_transport = None
        self.process_protocol = None

        # Process liveness monitoring
        self.liveness_transport = None
        self.liveness_protocol = None

        # Book-keeping for graph traversals
        self.reset_traversal_state()

    def reset_traversal_state(self):
        self.color = NodeColor.WHITE
        self.predecessor = None
        self.distance = sys.maxsize
        self.discovery_time = sys.maxsize
        self.finishing_time = sys.maxsize

    def __hash__(self):
        return hash(self.name)


class ProcessDirectedGraph(object):
    """
    Expected usage:

    1) Create the process DAG
    2) Make n calls to add_node
    3) Once all nodes have been added, call build() which extract adjacency information. As part of
       this process, we will uncover any references to topics which are only consumed but never produced,
       additional dependencies on processes which haven't been defined, etc.
    4) Call has_cycles -- this should return false to indicate that this is actually a (multi)-DAG
    5) Call topological_sort which does exactly what you'd expect
    """

    def __init__(self):
        super(ProcessDirectedGraph, self).__init__()
        self.nodes = {}
        self.edges = collections.defaultdict(list)
        self.logger = logging.getLogger('mercury.ProcessDirectedGraph')

    def add_node(self, managed_process_pb):
        """
        Given a ManagedProcess protobuf instance, create the corresponding ProcessNode and insert it into the graph.

        :param managed_process_pb: ManagedProcess protobuf instance
        :return:
        """
        node = ProcessNode(managed_process_pb)

        if node.name not in self.nodes:
            self.nodes[node.name] = node
            self.logger.info('Created node for [{}]'.format(node.name))
        else:
            self.logger.error(
                'Detected request to add a managed process using the name [{}] which is already taken'.format(
                    node.name))
            raise DuplicateManagedProcessName(
                'Cannot have more than one managed process with name [{}]'.format(node.name))

        return node

    def build(self):
        """
        Reset all adjacency information and rebuild it after adding nodes. This implements a collection of sanity checks
        (making sure every subscribed topic has a publisher, etc.) which, if they all pass, mean we can safely / sanely
        construct the dependency graph.
        """
        self.logger.info('Rebuilding adjacency information')
        self.edges = collections.defaultdict(list)

        topic_to_publisher = collections.defaultdict(list)
        topic_to_subscribers = collections.defaultdict(list)
        node_to_missing_deps = collections.defaultdict(list)

        result = True

        for node in self.nodes.values():
            for topic in node.provided_topics.keys():
                topic_to_publisher[topic].append(node)

            for topic in node.required_topics:
                topic_to_subscribers[topic].append(node)

            for dep in node.additional_dependencies:
                if dep not in self.nodes:
                    node_to_missing_deps[node].append(dep)

        if len(node_to_missing_deps) > 0:
            result = False
            msg = io.StringIO()
            print('Found [{}] managed processes with missing dependencies'.format(len(node_to_missing_deps)), file=msg)
            fmt = '   Managed process [{}] is missing [{}]'

            for (node, missing) in node_to_missing_deps.items():
                print(fmt.format(node.name, ', '.join(missing)), file=msg)
                self.logger.error(msg.getvalue())

        missing_publishers = []
        for topic in topic_to_subscribers.keys():
            if topic not in topic_to_publisher:
                missing_publishers.append(topic)

        if len(missing_publishers) > 0:
            result = False
            msg = io.StringIO()
            print('Found [{}] topics that do not have publishers'.format(len(missing_publishers)), file=msg)
            fmt = '  Topic [{}] with subscribers [{}]'

            for topic in missing_publishers:
                print(fmt.format(topic, ', '.join([x.name for x in topic_to_subscribers[topic]])), file=msg)
                self.logger.error(msg.getvalue())

        if not result:
            self.logger.error('Found errors when building adjacency information')
            raise GraphBuildError(
                'Found errors when building adjacency information / graph edges. Check log for details')

        # Now we have enough information to build our edges. Phase 1: pub/sub stuff
        for (topic, subscribers) in topic_to_subscribers.items():
            publishers = topic_to_publisher[topic]

            for p in publishers:
                for s in subscribers:
                    self.edges[p].append(s)

        # Phase 2: additional dependencies
        for node in self.nodes.values():
            for dep in node.additional_dependencies:
                src = self.nodes[dep]
                self.edges[src].append(node)

    def _reset_traversal_state(self):
        """
        Private. Resets the traversal state of all nodes in preparation for a new traversal
        :return:
        """
        for n in self.nodes.values():
            n.reset_traversal_state()

    def _visit_enter(self, node, visit_function):
        self.logger.info('Entering node [{}] with discovery time [{}]'.format(node.name, node.discovery_time))

        if visit_function is not None:
            visit_function(node, 'enter')

    def _visit_exit(self, node, visit_function):
        self.logger.info('Exiting node [{}] with finish time [{}]'.format(node.name, node.finishing_time))

        if visit_function is not None:
            visit_function(node, 'exit')

    def depth_first_traversal(self, visitor_function=None):
        """
        Perform a depth-first traversal of the entire graph. The supplied visitor_function callable is called twice per
        node: the first time as the node is entered (visitor_function(node, 'enter')) and the second as the node is
        exited (visitor_function(node, 'exit'))

        :param self:
        :param visitor_function: callable that takes two arguments, the first is the node being visited, and the second
        a flag indicating whether we are entering ('enter') or exiting ('exit') the node. Defaults to None
        :return: True if a cycle was detected, false otherwise
        """
        self._reset_traversal_state()
        self.time = 0

        result = False

        for n in self.nodes.values():
            if NodeColor.WHITE == n.color:
                stack = collections.deque()
                stack.append(n)

                while len(stack) > 0:
                    node = stack.pop()

                    if NodeColor.WHITE == node.color:
                        # Need to stay on the stack until we're done exploring things connected to this node
                        stack.append(node)

                        self.time += 1
                        node.discovery_time = self.time
                        self._visit_enter(node, visitor_function)
                        node.color = NodeColor.GRAY

                        for descendant in self.edges[node]:
                            self.logger.debug(
                                'Looking at [{}] -> [{} / {}]'.format(node.name, descendant.name, descendant.color))
                            if NodeColor.WHITE == descendant.color:
                                descendant.predecessor = node
                                stack.append(descendant)
                            elif NodeColor.GRAY == descendant.color:
                                self.logger.debug(
                                    'Found cycle involving edge [{}] -> [{}]'.format(node.name, descendant.name))
                                result = True

                    elif NodeColor.GRAY == node.color:
                        self.time += 1
                        node.color = NodeColor.BLACK
                        node.finishing_time = self.time
                        self._visit_exit(node, visitor_function)


            elif NodeColor.GRAY == n.color:
                self.logger.info('Found cycle involving node [{}]'.format(n.name))
                result = True

        return result

    def has_cycle(self):
        return self.depth_first_traversal()

    def topological_sort(self):
        sorted_nodes = []
        if self.depth_first_traversal(lambda x, y: sorted_nodes.extend([x] if y == 'exit' else [])):
            raise GraphIsCyclicError('Detected cycle in process graph')

        sorted_nodes.reverse()
        return sorted_nodes

    def breadth_first_traversal(self, start_node, visitor_function=None, max_depth=None):
        """
        Perform BFS, starting at the specified node.

        :param start_node: node from which the BFS should start. This can either be the node itself or a string
               which is the name of the node.
        :param visitor_function: callable that takes two arguments, the first is the node being visited, and the second
        a flag indicating whether we are entering ('enter') or exiting ('exit') the node. Defaults to None
        """
        self._reset_traversal_state()

        if isinstance(start_node, str):
            start_node = self.nodes[start_node]

        if not isinstance(start_node, ProcessNode):
            raise TypeError('Expect start_node to either be a string or a ProcessNode. Got [{}] instead'.format(
                str(type(start_node))))

        start_node.discovery_time = 1
        queue = collections.deque()
        queue.appendleft(start_node)

        while len(queue) > 0:
            node = queue.pop()
            assert NodeColor.WHITE == node.color

            if node.predecessor is not None:
                node.discovery_time = node.predecessor.discovery_time + 1

            self._visit_enter(node, visitor_function)

            node.color = NodeColor.GRAY

            if max_depth is None or node.discovery_time + 1 < max_depth:
                for descendant in self.edges[node]:
                    if NodeColor.WHITE == descendant:
                        descendant.predecessor = node
                        queue.appendleft(descendant)

            node.finishing_time = self.time
            node.color = NodeColor.BLACK

            self._visit_exit(node, visitor_function)
