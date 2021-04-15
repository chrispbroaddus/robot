#pragma once

#include <chrono>
#include <string.h>

#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/planning/definitions.h"

namespace planning {

using std::chrono::nanoseconds;
using hal::VCUCommandEnvelope;
using core::chrono::gps::wallClockInNanoseconds;

/**
 * @brief Validate the specified CommandEnvelope, using the same logic that is
 *        applied on the VCU itself.
 *
 *        Note: Ideally this should be in a separate library that is common
 *        across the codebases, but this is what we have currently.
 *
 * @param[in] command           Command envelope to validate
 * @param[out] error_reason     Success/Error state
 *
 * @return Command was successfully validated
 */
static bool commandValidate(const VCUCommandEnvelope& command, int& error_reason) {
    // Assume everything is OK.
    error_reason = 0;

    // Get the system time.
    auto systime = wallClockInNanoseconds();

    // Validate the sequence number.
    if (command.sequencenumber() == 0) {
        // Sequence number 0 is reserved for VCU use
        LOG(ERROR) << "COMMAND: sequence 0 is reserved for VCU use";
        error_reason = 2;
        return false;
    }
    // Here we validate commands.  We don't want to execute the command here.
    // Rather, we just want to make sure that the command is valid.
    switch (command.command_case()) {
    case VCUCommandEnvelope::kIdleCommand: {
        // No checking needed
        break;
    }

    case VCUCommandEnvelope::kEmergencyStopCommand: {
        // No checking needed
        break;
    }

    case VCUCommandEnvelope::kTrajectoryCommand: {
        // Point to the trajectory portion of the command.
        const auto& trajectory = command.trajectorycommand();

        // Check 1: There are a reasonable number of segments;
        if ((trajectory.segments_size() < 1)) {
            LOG(ERROR) << "COMMAND: invalid number of trajectory segments";
            error_reason = 4;
            break;
        }

        // Check 2: The trajectory terminates at zero velocity.
        if (trajectory.segments(trajectory.segments_size() - 1).arcdrive().linearvelocitymeterspersecond() != 0.0f) {
            LOG(ERROR) << "COMMAND: trajectory doesn't terminate at zero velocity";
            error_reason = 5;
            break;
        }

        // Check 3: All segment velocities and curvatures are in range.
        for (int i = 0; i < trajectory.segments_size(); i++) {
            if (trajectory.segments(i).arcdrive().linearvelocitymeterspersecond() < -PlannerConstants<float>::kHardSpeedLimit
                || trajectory.segments(i).arcdrive().linearvelocitymeterspersecond() > +PlannerConstants<float>::kHardSpeedLimit
                || trajectory.segments(i).arcdrive().curvatureinversemeters() < -PlannerConstants<float>::kCurvatureLimit
                || trajectory.segments(i).arcdrive().curvatureinversemeters() > +PlannerConstants<float>::kCurvatureLimit) {
                LOG(ERROR) << "COMMAND: trajectory velocities or curvatures failed range check";
                error_reason = 6;
                break;
            }
        }

        if (error_reason)
            break;

        // Check 4: The first segment is in the future:
        if (nanoseconds(trajectory.segments(0).targetstarttime().nanos()) <= systime) {
            // Determine milliseconds in the past.
            auto nanoseconds_past = systime - nanoseconds(trajectory.segments(0).targetstarttime().nanos());
            LOG(ERROR) << "COMMAND: the first trajectory segment is in the past by " << nanoseconds_past.count() << " nanoseconds";
            error_reason = 7;
            break;
        }

        // Check 5: The trajectory segments are in chronological order:
        for (int i = 1; i < trajectory.segments_size(); i++) {
            if (trajectory.segments(i).targetstarttime().nanos() <= trajectory.segments(i - 1).targetstarttime().nanos()) {
                LOG(ERROR) << "COMMAND: the trajectory segments are not in the chronological order";
                error_reason = 8;
                break;
            }
        }
        if (error_reason)
            break;

        // We made it!
        break;
    }

    default: {
        // Unknown command tag!
        LOG(ERROR) << "COMMAND: unknown command tag";
        error_reason = 9;
        break;
    }
    }
    // Return whether the command checked out as valid.
    return error_reason ? false : true;
}

} // planning
