#include "packages/object_tracking/include/state.h"

using namespace object_tracking;

std::shared_ptr<StateInterface> InitialCandidateState::transit(std::function<std::shared_ptr<StateInterface>()> func) {
    auto ret = func();
    if (!(dynamic_cast<ActiveDetectedState*>(ret.get()) || dynamic_cast<InactiveState*>(ret.get()))) {
        throw std::runtime_error("Wrong state transition.");
    }
    return std::move(ret);
}

std::shared_ptr<StateInterface> ActiveDetectedState::transit(std::function<std::shared_ptr<StateInterface>()> func) {
    auto ret = func();
    if (!(dynamic_cast<ActiveDetectedState*>(ret.get()) || dynamic_cast<ActiveLostState*>(ret.get()))) {
        throw std::runtime_error("Wrong state transition");
    }
    return std::move(ret);
}

std::shared_ptr<StateInterface> ActiveLostState::transit(std::function<std::shared_ptr<StateInterface>()> func) {
    auto ret = func();
    if (!(dynamic_cast<ActiveDetectedState*>(ret.get()) || dynamic_cast<ActiveLostState*>(ret.get())
            || dynamic_cast<InactiveState*>(ret.get()))) {
        throw std::runtime_error("Wrong state transition");
    }
    return std::move(ret);
}

std::shared_ptr<StateInterface> InactiveState::transit(std::function<std::shared_ptr<StateInterface>()> func) {
    auto ret = func();
    if (!(dynamic_cast<InactiveState*>(ret.get()))) {
        throw std::runtime_error("Wrong state transition");
    }
    return std::move(ret);
}
