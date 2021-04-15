import logging
import re
import signal
import unittest
import unittest.mock
from applications.mercury.lib.recovery_policy import *


class CommonFixture(unittest.TestCase):
    """
    Common test fixture methods for testing individual recovery states
    """
    pass


class StateMachineTransitionTest(CommonFixture):
    """
    Verify all the state transitions behave as expected
    """

    def check_error_events(self, state):
        self.assertIsInstance(state.next_state(RecoveryEvent.PROCESS_EXITED), CatastrophicFailureState)
        self.assertIsInstance(state.next_state(RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED), CatastrophicFailureState)

    def check_unhandled_events(self, state, unhandled_events):
        for e in unhandled_events:
            with self.assertRaises(UnexpectedEvent):
                state.next_state(e)

    def test_next_state_catastrophic_failure(self):
        state = CatastrophicFailureState()

        # Catastrophic failure is a sink
        for evt in RecoveryEvent:
            self.assertIs(state.next_state(evt), state)

    def test_next_state_initial_state_handled_events(self):
        state = InitialState()
        self.assertIsInstance(state.next_state(RecoveryEvent.REQUEST_LAUNCH_PROCESSES), LaunchingProcessesState)

    def test_next_state_initial_state_unhandled_events(self):
        handled_events = {RecoveryEvent.REQUEST_LAUNCH_PROCESSES}

        unhandled_events = frozenset(RecoveryEvent) - handled_events
        state = InitialState()
        self.check_unhandled_events(state, unhandled_events)

    def test_next_state_launching_processes_handled_events(self):
        state = LaunchingProcessesState()
        self.check_error_events(state)
        self.assertIsInstance(state.next_state(RecoveryEvent.PROCESSES_LAUNCHED), NominalState)

    def test_next_state_launching_processes_unhandled_events(self):
        handled_events = {RecoveryEvent.PROCESS_EXITED, RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                          RecoveryEvent.PROCESSES_LAUNCHED}
        unhandled_events = frozenset(RecoveryEvent) - handled_events
        state = LaunchingProcessesState()
        self.check_unhandled_events(state, unhandled_events)

    def test_next_state_nominal_state_handled_events(self):
        state = NominalState()
        self.assertIsInstance(state.next_state(RecoveryEvent.PROCESS_EXITED), AttemptRecoveryState)
        self.assertIsInstance(state.next_state(RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED), AttemptRecoveryState)

    def test_next_state_nominal_state_unhandled_events(self):
        handled_events = {RecoveryEvent.PROCESS_EXITED, RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED}
        unhandled_events = frozenset(RecoveryEvent) - handled_events
        state = NominalState()
        self.check_unhandled_events(state, unhandled_events)

    def test_next_state_attempt_recovery_state_handled_events(self):
        state = AttemptRecoveryState()

        self.check_error_events(state)
        self.assertIsInstance(state.next_state(RecoveryEvent.CATASTROPHIC_FAILURE), CatastrophicFailureState)
        self.assertIsInstance(state.next_state(RecoveryEvent.FAILURE_IGNORED), NominalState)
        self.assertIsInstance(state.next_state(RecoveryEvent.REQUEST_STOP_SUBGRAPH), RequestStopAffectedProcessesState)

    def test_next_state_attempt_recovery_state_unhandled_events(self):
        handled_events = {RecoveryEvent.PROCESS_EXITED,
                          RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                          RecoveryEvent.CATASTROPHIC_FAILURE,
                          RecoveryEvent.FAILURE_IGNORED,
                          RecoveryEvent.REQUEST_STOP_SUBGRAPH}
        unhandled_events = frozenset(RecoveryEvent) - handled_events
        state = AttemptRecoveryState()
        self.check_unhandled_events(state, unhandled_events)

    def test_next_state_request_stop_affected_processes_handled_events(self):
        state = RequestStopAffectedProcessesState()
        self.check_error_events(state)
        self.assertIsInstance(state.next_state(RecoveryEvent.CATASTROPHIC_FAILURE), CatastrophicFailureState)
        self.assertIsInstance(state.next_state(RecoveryEvent.FORCE_STOP_SUBGRAPH), ForceStopAffectedProcessesState)

    def test_next_state_requst_stop_affected_processes_unhandled_events(self):
        handled_events = {RecoveryEvent.CATASTROPHIC_FAILURE,
                          RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                          RecoveryEvent.PROCESS_EXITED,
                          RecoveryEvent.FORCE_STOP_SUBGRAPH}
        unhandled_events = frozenset(RecoveryEvent) - handled_events
        state = RequestStopAffectedProcessesState()
        self.check_unhandled_events(state, unhandled_events)

    def test_next_state_force_stop_affected_processes_handled_events(self):
        state = ForceStopAffectedProcessesState()
        self.check_error_events(state)
        self.assertIsInstance(state.next_state(RecoveryEvent.CATASTROPHIC_FAILURE), CatastrophicFailureState)
        self.assertIsInstance(state.next_state(RecoveryEvent.RESTART_FAILED_PROCESSES), RestartAffectedProcessesState)

    def test_next_state_force_stop_affected_processes_unhandled_events(self):
        handled_events = {RecoveryEvent.PROCESS_EXITED,
                          RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                          RecoveryEvent.CATASTROPHIC_FAILURE,
                          RecoveryEvent.RESTART_FAILED_PROCESSES}
        unhandled_events = frozenset(RecoveryEvent) - handled_events
        state = ForceStopAffectedProcessesState()
        self.check_unhandled_events(state, unhandled_events)

    def test_next_state_restart_affected_processes_handled_events(self):
        state = RestartAffectedProcessesState()
        self.check_error_events(state)
        self.assertIsInstance(state.next_state(RecoveryEvent.CATASTROPHIC_FAILURE), CatastrophicFailureState)
        self.assertIsInstance(state.next_state(RecoveryEvent.RELAUNCH_COMPLETE), NominalState)

    def test_next_state_restart_affected_processes_unhandled_events(self):
        handled_events = {RecoveryEvent.PROCESS_EXITED,
                          RecoveryEvent.PROCESS_LIVENESS_CHECK_FAILED,
                          RecoveryEvent.CATASTROPHIC_FAILURE,
                          RecoveryEvent.RELAUNCH_COMPLETE}
        unhandled_events = frozenset(RecoveryEvent) - handled_events
        state = RestartAffectedProcessesState()
        self.check_unhandled_events(state, unhandled_events)
