import asyncio
import enum
import functools
import logging
import time
from applications.mercury.lib.protocols import *


class RecoveryEvent(enum.Enum):
    REQUEST_LAUNCH_PROCESSES = 1
    PROCESS_EXITED = 2
    PROCESS_LIVENESS_CHECK_FAILED = 3
    PROCESSES_LAUNCHED = 4
    FAILURE_IGNORED = 5
    REQUEST_STOP_SUBGRAPH = 6
    CATASTROPHIC_FAILURE = 7
    FORCE_STOP_SUBGRAPH = 8
    RESTART_FAILED_PROCESSES = 9
    RELAUNCH_COMPLETE = 10


class UnexpectedEvent(RuntimeError):
    """
    Thrown when a recovery policy state sees an unexpected event
    """
    pass


class RecoveryContext(object):
    """
    Struct passed to the the action method of RecoveryPolicyState instances. This should contain everything
    needed for those states to act (e.g. to start launching processes, kill processes, etc.)
    """

    def __init__(self, process_node, process_graph, recovery_policy, event_loop):
        self.__process_node = process_node
        self.__process_graph = process_graph
        self.__event_loop = event_loop
        self.__recovery_policy = recovery_policy

    @property
    def process_node(self):
        return self.__process_node

    @property
    def process_graph(self):
        return self.__process_graph

    @property
    def event_loop(self):
        return self.__event_loop

    @property
    def recovery_policy(self):
        return self.__recovery_policy


class RecoveryPolicyState(object):
    """
    Common base class for all states in the recovery policy state machine.
    """

    def __init__(self, name):
        self.__name = name
        self.logger = logging.getLogger('mercury.Recovery[{}]'.format(name))

    @property
    def name(self):
        return self.__name

    def _unhandled_event(self, event):
        """
        (protected)

        Convenience method for dealing with unhandled events
        :param event: the unhandled event
        :return:
        """
        fmt = 'Unexpected event [{}]'
        msg = fmt.format(event, self.name)
        self.logger.error(msg)
        raise UnexpectedEvent(msg)

    @asyncio.coroutine
    def action(self, recovery_context):
        """
        Overridden by subclasses. This method is called once, as a state is entered. It is provided a recovery context
        to facilitate any actions that must be taken upon entry into this state. When finished, subclasses may either
        return a RecoveryEvent (signalling there is some event which the state machine must react to) or None (signalling
        no further events)

        Note: this should be a coroutine.
        :param recovery_context:
        """
        pass

    def next_state(self, event):
        """
        Given an event, return the next state. Note that this method may return self if a self-transition is requested.
        :param event:
        :return:
        """
        pass


class CatastrophicFailureState(RecoveryPolicyState):
    """
    Deal with catastrophic failure. For now, this just acts as a sink state. In the future, this will be enhanced to signal
    to the VCU that we believe we are in a catastrophic failure state.
    """

    def __init__(self):
        super(CatastrophicFailureState, self).__init__('Catastrophic failure')

    @asyncio.coroutine
    def action(self, recovery_context):
        assert recovery_context.process_graph is not None
        assert recovery_context.event_loop is not None
        nodes = reversed(recovery_context.process_graph.topological_sort())

        # Step 1: disable events and request stop
        wait_time = 10
        for node in nodes:
            self.logger.info('Disabling events for [{}]'.format(node.name))
            node.process_protocol.disable_events()

            self.logger.info('Requesting that process [{}] stop'.format(node.name))
            node.process_protocol.request_stop()

            self.logger.info('Scheduling force stop of process [{}] in [{}] seconds'.format(node.name, wait_time))
            recovery_context.event_loop.call_later(wait_time, lambda x=node: x.process_protocol.force_stop())

        # Step 5: (TODO)
        # Engage soft e-stop mechanism once we know what that is and how it works

        self.logger.info('Scheduling cancelling event loop')
        recovery_context.event_loop.call_later(wait_time * 2, functools.partial(recovery_context.event_loop.stop))
        return None

    def next_state(self, event):
        return self


class InitialState(RecoveryPolicyState):
    """
    Initial state
    """

    def __init__(self):
        super(InitialState, self).__init__('Initial state')

    @asyncio.coroutine
    def action(self, recovery_context):
        raise RuntimeError('Should never be called')

    def next_state(self, event):
        if event == RecoveryEvent.REQUEST_LAUNCH_PROCESSES:
            return LaunchingProcessesState()
        else:
            self._unhandled_event(event)


def _process_protocol_factory(graph_node, recovery_policy):
    if graph_node.process_protocol is None:
        return ProcessWatcherProtocol(graph_node, recovery_policy)
    else:
        graph_node.process_protocol.reset()
        return graph_node.process_protocol


@asyncio.coroutine
def _launch_process(loop, process, policy):
    logger = logging.getLogger('mercury.Recovery')
    logger.info('Launching [{}] (executable: [{}], arguments: [{}])'.format(process.name,
                                                                            process.executable_path,
                                                                            ', '.join(['\'{}\''.format(y) for y in
                                                                                       process.arguments])))

    if process.logfile:
        log = open(process.logfile, 'a+')
        coro = loop.subprocess_exec(lambda: _process_protocol_factory(process, policy), process.executable_path,
                                    *process.arguments, stdout=log, stderr=log)
    else:
        coro = loop.subprocess_exec(lambda: _process_protocol_factory(process, policy), process.executable_path,
                                    *process.arguments)
    future = asyncio.ensure_future(coro, loop=loop)
    (process.process_transport, process.process_protocol) = yield from future


@asyncio.coroutine
def _launch_process_graph(loop, graph, policy):
    logger = logging.getLogger('mercury.Recovery')

    logger.info('Determining process launch order via topological sort')
    to_visit = graph.topological_sort()

    scheduled_time = 1
    wait_between_launches = 5
    for x in to_visit:
        logger.info('Scheduling launching [{}] in approximately [{}] seconds'.format(x.name, scheduled_time))
        loop.call_later(scheduled_time,
                        lambda node=x: asyncio.ensure_future(_launch_process(loop, node, policy), loop=loop))
        scheduled_time += wait_between_launches

    yield from asyncio.sleep(scheduled_time + 1, loop=loop)
    logger.info('All processes launched')


class LaunchingProcessesState(RecoveryPolicyState):
    """
    Doing the initial launch of all our processes.
    """

    def __init__(self):
        super(LaunchingProcessesState, self).__init__('Launching processes')

    @asyncio.coroutine
    def action(self, recovery_context):
        assert recovery_context.event_loop is not None
        assert recovery_context.process_graph is not None
        yield from _launch_process_graph(recovery_context.event_loop, recovery_context.process_graph,
                                         recovery_context.recovery_policy)
        self.logger.info('Returning PROCESSES_LAUNCHED')
        return RecoveryEvent.PROCESSES_LAUNCHED

    def next_state(self, event):
        if event == RecoveryEvent.PROCESSES_LAUNCHED:
            return NominalState()
        elif event == RecoveryEvent.PROCESS_EXITED or event == RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED:
            return CatastrophicFailureState()
        else:
            self._unhandled_event(event)


class NominalState(RecoveryPolicyState):
    """
    Nominal state. While in this state, hypothetically everything is good.
    """

    def __init__(self):
        super(NominalState, self).__init__('Nominal state')

    @asyncio.coroutine
    def action(self, recovery_context):
        return None

    def next_state(self, event):
        if event == RecoveryEvent.PROCESS_EXITED or event == RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED:
            return AttemptRecoveryState()
        else:
            self._unhandled_event(event)


class AttemptRecoveryState(RecoveryPolicyState):
    """
    Attempt a recovery
    """

    def __init__(self):
        super(AttemptRecoveryState, self).__init__('Attempting recovery')

    @asyncio.coroutine
    def action(self, recovery_context):

        # Verify pre-conditions
        assert recovery_context.process_node is not None
        assert recovery_context.recovery_policy is not None
        assert recovery_context.process_graph is not None
        assert recovery_context.event_loop is not None

        # Look at the policy associated with the failed node and then figure out what to do
        if recovery_context.process_node.ignore_failures:
            self.logger.info('Process [{}] has a policy of ignore failures.'.format(recovery_context.process_node.name))
            return RecoveryEvent.FAILURE_IGNORED
        elif recovery_context.process_node.failure_is_catastrophic:
            self.logger.info('Process [{}] has a policy of treating failures as catastrophic.'.format(
                recovery_context.process_node.name))
            return RecoveryEvent.CATASTROPHIC_FAILURE
        else:
            self.logger.info(
                'Process [{}] has a policy of attempt to recovery..'.format(recovery_context.process_node.name))
            return RecoveryEvent.REQUEST_STOP_SUBGRAPH

    def next_state(self, event):
        if event in {RecoveryEvent.PROCESS_EXITED, RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                     RecoveryEvent.CATASTROPHIC_FAILURE}:
            return CatastrophicFailureState()
        elif event == RecoveryEvent.FAILURE_IGNORED:
            return NominalState()
        elif event == RecoveryEvent.REQUEST_STOP_SUBGRAPH:
            return RequestStopAffectedProcessesState()
        else:
            self._unhandled_event(event)


class RequestStopAffectedProcessesState(RecoveryPolicyState):
    """
    Perform a BFS and shut-down components in the order in which nodes are colored black (up-to fixed depth).
    """

    def __init__(self):
        super(RequestStopAffectedProcessesState, self).__init__('Request that affected processes stop')

    @asyncio.coroutine
    def action(self, recovery_context):
        # Verify pre-conditions
        assert recovery_context.process_node is not None
        assert recovery_context.recovery_policy is not None
        assert recovery_context.process_graph is not None
        assert recovery_context.event_loop is not None
        max_depth = 0 if recovery_context.process_node.propagate_to_descendants else None

        # As we enter nodes, disable their events, and as we exit, request they stop
        def visitor(node, direction):
            # TODO
            # When we add liveness monitoring, the liveness protocol will also need to have its events disabled.
            if 'enter' == direction:
                node.process_protocol.disable_events()
            if 'exit' == direction:
                node.process_protocol.request_stop()

        recovery_context.process_graph.breadth_first_traversal(recovery_context.process_node, visitor, max_depth)
        return RecoveryEvent.FORCE_STOP_SUBGRAPH

    def next_state(self, event):
        if event in {RecoveryEvent.PROCESS_EXITED, RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                     RecoveryEvent.CATASTROPHIC_FAILURE}:
            return CatastrophicFailureState()
        elif event == RecoveryEvent.FORCE_STOP_SUBGRAPH:
            return ForceStopAffectedProcessesState()
        else:
            self._unhandled_event(event)


class ForceStopAffectedProcessesState(RecoveryPolicyState):
    """
    Perform a BFS and shut-down components in the order in which nodes are colored black (up-to fixed depth).
    """

    def __init__(self):
        super(ForceStopAffectedProcessesState, self).__init__('Force affected processes to stop')

    @asyncio.coroutine
    def action(self, recovery_context):
        # Verify pre-conditions
        assert recovery_context.process_node is not None
        assert recovery_context.recovery_policy is not None
        assert recovery_context.process_graph is not None
        assert recovery_context.event_loop is not None
        max_depth = 0 if recovery_context.process_node.propagate_to_descendants else None

        # As we enter nodes, disable their events, and as we exit, request they stop
        def visitor(node, direction):
            if 'exit' == direction:
                node.process_protocol.force_stop()

        recovery_context.process_graph.breadth_first_traversal(recovery_context.process_node, visitor, max_depth)
        return RecoveryEvent.RESTART_FAILED_PROCESSES

    def next_state(self, event):
        if event in {RecoveryEvent.PROCESS_EXITED, RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                     RecoveryEvent.CATASTROPHIC_FAILURE}:
            return CatastrophicFailureState()
        elif event == RecoveryEvent.RESTART_FAILED_PROCESSES:
            return RestartAffectedProcessesState()
        else:
            self._unhandled_event(event)


class RestartAffectedProcessesState(RecoveryPolicyState):
    """
    Restart the affected subgraph after it has been shut down.
    """

    def __init__(self):
        super(RestartAffectedProcessesState, self).__init__('Restarting affected processes')

    @asyncio.coroutine
    def action(self, recovery_context):
        # Verify pre-conditions
        assert recovery_context.process_node is not None
        assert recovery_context.recovery_policy is not None
        assert recovery_context.process_graph is not None
        assert recovery_context.event_loop is not None
        max_depth = 0 if recovery_context.process_node.propagate_to_descendants else None

        def relaunch(loop, node, policy):
            yield from _launch_process(loop, node, policy)
            node.process_protocol.enable_events()

        # As we enter nodes, relaunch the processes and re-enable their events
        wait_between_launches = 5
        max_time = 0

        def visitor(node, direction):
            nonlocal max_time
            if 'enter' == direction:
                if max_time < node.discovery_time:
                    max_time = node.discovery_time

                # Launch time is 1 (so that we guarantee that the request takes place in the future when we do call_later
                # below) plus the current wave * 5. This will cause everything that can be relaunched together to do so
                # in parallel, with 5 seconds between each wave.
                launch_time = 1 + node.discovery_time * wait_between_launches
                node.process_protocol.reset()

                recovery_context.event_loop.call_later(launch_time,
                                                       lambda: asyncio.ensure_future(relaunch(recovery_context.event_loop, node, recovery_context.recovery_policy), loop=recovery_context.event_loop))

        recovery_context.process_graph.breadth_first_traversal(recovery_context.process_node, visitor, max_depth)

        # Sleep one second after the last process was launched and then return.
        yield from asyncio.sleep(2 + wait_between_launches * max_time)
        return RecoveryEvent.RELAUNCH_COMPLETE

    def next_state(self, event):
        if event in {RecoveryEvent.PROCESS_EXITED, RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                     RecoveryEvent.CATASTROPHIC_FAILURE}:
            return CatastrophicFailureState()
        elif event == RecoveryEvent.RELAUNCH_COMPLETE:
            return NominalState()
        else:
            self._unhandled_event(event)


class RecoveryPolicy(object):
    """
    Encapsulates all the logic necessary to attempt 
    """

    def __init__(self, process_graph, event_loop):
        self.logger = logging.getLogger("mercury.RecoveryPolicy")
        self.state = InitialState()
        self.process_graph = process_graph
        self.event_loop = event_loop
        self.state_lock = asyncio.Lock(loop=event_loop)

    def _step_state_machine(self, process, event):
        ctx = RecoveryContext(process, self.process_graph, self, self.event_loop)

        while event is not None:
            self.logger.info('Stepping state machine through event [{}]'.format(event))

            # Note:
            #
            # This fine-grained locking is important. This gives every source of events (including unexpected failures
            # while in the midst of carrying out recovery actions) a fair chance to be considered, rather than having
            # states with long-running chains of actions blocking everything else out indefinitely.
            with (yield from self.state_lock):
                self.logger.info('Handling event [{}] while in state [{}]'.format(event, self.state.name))
                state = self.state.next_state(event)

                # Unfortunately, we need to hold onto the lock while performing the action associated with a state to
                # ensure that there are only at most one actions occuring concurrently.
                if state is not self.state:
                    self.logger.info('Transitioning from state [{}] to state [{}]'.format(self.state.name, state.name))
                    self.state = state

                    self.logger.info('Performing action for state [{}]'.format(self.state.name))
                    event = yield from self.state.action(ctx)
                else:
                    self.logger.info('State [{}] remains unchanged'.format(self.state.name))
                    event = None

    @asyncio.coroutine
    def launch_processes(self):
        self.logger.info('[launch_processes] Sending REQUEST_LAUNCH_PROCESS event')
        yield from self._step_state_machine(None, RecoveryEvent.REQUEST_LAUNCH_PROCESSES)

    def notify_process_exited(self, graph_node, return_code):
        self.logger.info('[notify_process_exited] Sending PROCESS_EXITED')
        asyncio.ensure_future(self._step_state_machine(graph_node, RecoveryEvent.PROCESS_EXITED), loop=self.event_loop)
