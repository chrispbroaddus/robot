#include "packages/net/include/zmq_topic_pub.h"
#include "packages/teleop/proto/vehicle_message.pb.h"

namespace docking {

///
/// Publish docking status
///
class DockingStatusPublisher {
public:
    DockingStatusPublisher(
        const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic)
        : mContext(1)
        , mPublisher(mContext, serverAddress, highWaterMark, lingerPeriodInMilliseconds)
        , mTopic(topic) {}

    DockingStatusPublisher(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds)
        : DockingStatusPublisher(serverAddress, highWaterMark, lingerPeriodInMilliseconds, "docking_status") {}

    void publish(const teleop::DockingStatus& status) { mPublisher.send(status, mTopic); }

private:
    zmq::context_t mContext;
    net::ZMQProtobufPublisher<teleop::DockingStatus> mPublisher;
    std::string mTopic;
};
}
