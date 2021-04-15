#pragma once

#include <functional>
#include <memory>
#include <stdexcept>

namespace object_tracking {

///
/// Base virtual class to define states for object tracking
///
class StateInterface {

public:
    /// Safe transition between states, making sure that the transition is well-defined
    virtual std::shared_ptr<StateInterface> transit(std::function<std::shared_ptr<StateInterface>()> func) = 0;
};

///
/// \brief The initial state for a detected object candidate.
///        The state can transit to i) ActiveDetectedState or ii) InactiveState
///
class InitialCandidateState : public StateInterface {
public:
    InitialCandidateState(){};
    std::shared_ptr<StateInterface> transit(std::function<std::shared_ptr<StateInterface>()> func);
};

///
/// \brief The state for actively tracked object over last a few frames & detected at the current frame
///        The state can transit i) again to ActiveDetectedState or ii) to ActiveLostState
///
class ActiveDetectedState : public StateInterface {
public:
    ActiveDetectedState() {}
    std::shared_ptr<StateInterface> transit(std::function<std::shared_ptr<StateInterface>()> func);
};

///
/// \brief The state for actively tracked object over last a few frames, but not detected at current frame.
///        The state can transit i) back to ActiveDetectedState, ii) again to ActiveLostState, or iii) becomes inactive
///
class ActiveLostState : public StateInterface {
public:
    ActiveLostState() {}
    std::shared_ptr<StateInterface> transit(std::function<std::shared_ptr<StateInterface>()> func);
};

///
/// \brief The state for not actively tracked object, either disappeared or occluded too long.
///        The state cannot transit to other state than itself.
///
class InactiveState : public StateInterface {
public:
    InactiveState() {}
    std::shared_ptr<StateInterface> transit(std::function<std::shared_ptr<StateInterface>()> func);
};
}
