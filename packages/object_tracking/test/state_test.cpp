#include "packages/object_tracking/include/state.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

using namespace object_tracking;

std::unique_ptr<InitialCandidateState> staticTransitionToInitialCandidateState();
std::unique_ptr<ActiveDetectedState> staticTransitionToActiveDetectedState();
std::unique_ptr<ActiveLostState> staticTransitionToActiveLostState();
std::unique_ptr<InactiveState> staticTransitionToInactiveState();

std::unique_ptr<InitialCandidateState> staticTransitionToInitialCandidateState() {
    std::unique_ptr<InitialCandidateState> state(new InitialCandidateState());
    return state;
}
std::unique_ptr<ActiveDetectedState> staticTransitionToActiveDetectedState() {
    std::unique_ptr<ActiveDetectedState> state(new ActiveDetectedState());
    return state;
}
std::unique_ptr<ActiveLostState> staticTransitionToActiveLostState() {
    std::unique_ptr<ActiveLostState> state(new ActiveLostState());
    return state;
}
std::unique_ptr<InactiveState> staticTransitionToInactiveState() {
    std::unique_ptr<InactiveState> state(new InactiveState());
    return state;
}

class TestInitialCandidateState : public InitialCandidateState {
public:
    TestInitialCandidateState() {}
};

TEST(StateMachine, transitionFromInitialCandidateState) {
    TestInitialCandidateState s1;
    EXPECT_THROW(s1.transit(staticTransitionToInitialCandidateState), std::runtime_error);
    TestInitialCandidateState s2;
    EXPECT_NO_THROW(s2.transit(staticTransitionToActiveDetectedState));
    TestInitialCandidateState s3;
    EXPECT_THROW(s3.transit(staticTransitionToActiveLostState), std::runtime_error);
    TestInitialCandidateState s4;
    EXPECT_NO_THROW(s4.transit(staticTransitionToInactiveState));
}

class TestActiveDetectedState : public ActiveDetectedState {
public:
    TestActiveDetectedState() {}
};

TEST(StateMachine, transitionFromActiveDetectedState) {
    TestActiveDetectedState s1;
    EXPECT_THROW(s1.transit(staticTransitionToInitialCandidateState), std::runtime_error);
    TestActiveDetectedState s2;
    EXPECT_NO_THROW(s2.transit(staticTransitionToActiveDetectedState));
    TestActiveDetectedState s3;
    EXPECT_NO_THROW(s3.transit(staticTransitionToActiveLostState));
    TestActiveDetectedState s4;
    EXPECT_THROW(s4.transit(staticTransitionToInactiveState), std::runtime_error);
}

class TestActiveLostState : public ActiveLostState {
public:
    TestActiveLostState() {}
};

TEST(StateMachine, transitionFromActiveLostState) {
    TestActiveLostState s1;
    EXPECT_THROW(s1.transit(staticTransitionToInitialCandidateState), std::runtime_error);
    TestActiveLostState s2;
    EXPECT_NO_THROW(s2.transit(staticTransitionToActiveDetectedState));
    TestActiveLostState s3;
    EXPECT_NO_THROW(s3.transit(staticTransitionToActiveLostState));
    TestActiveLostState s4;
    EXPECT_NO_THROW(s4.transit(staticTransitionToInactiveState));
}

class TestInactiveState : public InactiveState {
public:
    TestInactiveState() {}
};

TEST(StateMachine, transitionFromInactiveState) {
    TestInactiveState s1;
    EXPECT_THROW(s1.transit(staticTransitionToInitialCandidateState), std::runtime_error);
    TestInactiveState s2;
    EXPECT_THROW(s2.transit(staticTransitionToActiveDetectedState), std::runtime_error);
    TestInactiveState s3;
    EXPECT_THROW(s3.transit(staticTransitionToActiveLostState), std::runtime_error);
    TestInactiveState s4;
    EXPECT_NO_THROW(s4.transit(staticTransitionToInactiveState));
}
