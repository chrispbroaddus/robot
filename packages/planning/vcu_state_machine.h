#include "packages/hal/proto/vcu_trajectory_command.pb.h"

#include "boost/msm/back/state_machine.hpp"
#include "boost/msm/front/state_machine_def.hpp"
#include "glog/logging.h"

using namespace std;
namespace msm = boost::msm;
using namespace msm::front;
namespace mpl = boost::mpl;

namespace planning {
namespace interop {

    class ZippyStateMachine_ : public boost::msm::front::state_machine_def<ZippyStateMachine_> {
    public:
        struct Uninitialized : public boost::msm::front::state<> {
            template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "UNINITIALIZED"; }
        };

        typedef Uninitialized initial_state;

        struct Stationary : public boost::msm::front::state<> {
            template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "Stationary"; }
        };

        struct ArcDrive : public boost::msm::front::state<> {
            template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "ArcDrive"; }
        };

        struct TurnInPlace : public boost::msm::front::state<> {
            template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "TurnInPlace"; }
        };

        struct initialize {};
        void do_initialization(__attribute__((unused)) initialize const& event) {}

        struct turnInPlace {};
        void do_turnInPlace(__attribute__((unused)) turnInPlace const& event) {}

        struct arcDrive {};
        void do_arcDrive(__attribute__((unused)) arcDrive const& event) {}

        template <class FSM, class Event> void no_transition(__attribute__((unused)) Event const& event, FSM&, int) {
            std::string error = "no_transition called!";
            LOG(ERROR) << error;
            throw std::runtime_error(error);
        }

        template <class FSM, class Event> void exception_caught(Event const&, FSM&, std::exception& exception) {
            throw std::runtime_error(exception.what());
        }

        struct transition_table : boost::mpl::vector<a_row<Uninitialized, initialize, Stationary, &ZippyStateMachine_::do_initialization>,
                                      a_row<Stationary, turnInPlace, TurnInPlace, &ZippyStateMachine_::do_turnInPlace>,
                                      a_row<TurnInPlace, turnInPlace, TurnInPlace, &ZippyStateMachine_::do_turnInPlace>,
                                      a_row<Stationary, arcDrive, ArcDrive, &ZippyStateMachine_::do_arcDrive>,
                                      a_row<ArcDrive, arcDrive, ArcDrive, &ZippyStateMachine_::do_arcDrive> > {};
    };

    typedef boost::msm::back::state_machine<ZippyStateMachine_> ZippyStateMachine;

    class ZippyStateMachineDriver {
    public:
        ZippyStateMachineDriver() { m_stateMachine.process_event(ZippyStateMachine_::initialize()); }

        void validate(const hal::VCUTrajectoryCommand& trajectory) {
            for (int i = 0; i < trajectory.segments_size(); ++i) {
                const auto& command = trajectory.segments(i);
                if (command.has_arcdrive()) {
                    m_stateMachine.process_event(ZippyStateMachine_::arcDrive{});
                    continue;
                }
                if (command.has_turninplace()) {
                    m_stateMachine.process_event(ZippyStateMachine_::turnInPlace{});
                    continue;
                }
                LOG(ERROR) << trajectory.DebugString();
                throw std::runtime_error("Cannot process command!");
            }
        }

    private:
        ZippyStateMachine m_stateMachine;
    };

} // interop
} // planning
