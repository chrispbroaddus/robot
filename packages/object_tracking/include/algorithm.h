#pragma once

#include "state.h"
#include "track.h"

namespace object_tracking {

///
/// \brief Base class for the object tracking algorithm
///        Algorithm is the collection of track() functions for different states
///        Any tracking algorithm decide the next state, based on a history,
///        which is a collection of metadata of the current and previous frames.
///
template <class T> class TrackingAlgorithm {
public:
    static std::shared_ptr<StateInterface> track(
        const std::shared_ptr<StateInterface>& state, const std::queue<std::shared_ptr<TrackMetadataFrame> >& frames) {
        if (dynamic_cast<InitialCandidateState*>(state.get())) {
            return trackFromInitialCandidateStateBase(state, frames);
        } else if (dynamic_cast<ActiveDetectedState*>(state.get())) {
            return trackFromActiveDetectedStateBase(state, frames);
        } else if (dynamic_cast<ActiveLostState*>(state.get())) {
            return trackFromActiveLostStateBase(state, frames);
        } else if (dynamic_cast<InactiveState*>(state.get())) {
            return trackFromInactiveStateBase(state, frames);
        } else {
            InactiveState defaultInactiveState;
            return std::make_shared<InactiveState>(defaultInactiveState);
        }
    }

    static std::shared_ptr<StateInterface> trackFromInitialCandidateStateBase(
        const std::shared_ptr<StateInterface> state, const std::queue<std::shared_ptr<TrackMetadataFrame> >& frames) {
        return T::trackFromInitialCandidateState(state, frames);
    }

    static std::shared_ptr<StateInterface> trackFromActiveDetectedStateBase(
        const std::shared_ptr<StateInterface> state, const std::queue<std::shared_ptr<TrackMetadataFrame> >& frames) {
        return T::trackFromActiveDetectedState(state, frames);
    }

    static std::shared_ptr<StateInterface> trackFromActiveLostStateBase(
        const std::shared_ptr<StateInterface> state, const std::queue<std::shared_ptr<TrackMetadataFrame> >& frames) {
        return T::trackFromActiveLostState(state, frames);
    }

    static std::shared_ptr<StateInterface> trackFromInactiveStateBase(
        const std::shared_ptr<StateInterface> state, const std::queue<std::shared_ptr<TrackMetadataFrame> >& frames) {
        return T::trackFromInactiveState(state, frames);
    }
};
}
