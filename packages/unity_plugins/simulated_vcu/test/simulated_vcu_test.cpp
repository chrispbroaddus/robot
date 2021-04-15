#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/unity_plugins/simulated_vcu/include/simulated_vcu.h"

using namespace unity_plugins;

using core::test::UniquePortProvider;
UniquePortProvider provider;

TEST(SimulatedVCU, construct_n_destruct) {
    const int portNumber = provider.next_port();
    constexpr float PIDKu = 500;
    constexpr float PIDTu = 0.3;

    VehicleCalibration vehicleCalibration;
    std::string address = "tcp://127.0.0.1:" + std::to_string(portNumber);
    SimulatedVCU simulatedVCU(address, PIDKu, PIDTu, vehicleCalibration);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
