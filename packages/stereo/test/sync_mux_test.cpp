#include "packages/filter_graph/include/transform_filter.h"
#include "packages/stereo/include/sync_mux_filter.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

using namespace core;
using namespace hal;
using namespace stereo;
using namespace filter_graph;

namespace stereo_test {

class TestFilter : public filter_graph::TransformFilter {
public:
    TestFilter(const std::string& leftStreamId, const std::string& rightStreamId, std::vector<std::pair<int64_t, int64_t> > times)
        : filter_graph::TransformFilter("TestFilter", 10, 10)
        , m_leftStreamId(leftStreamId)
        , m_rightStreamId(rightStreamId)
        , m_times(times)
        , m_counter(0){};

    ~TestFilter(){};

    void receive(std::shared_ptr<filter_graph::Container> container) override {
        auto leftSampleOut = container->get(m_leftStreamId);
        if (leftSampleOut.get()) {
            auto leftCameraSampleOut = std::static_pointer_cast<filter_graph::AggregateSample<hal::CameraSample> >(leftSampleOut);
            EXPECT_EQ(m_times[m_counter].first, leftCameraSampleOut->data().systemtimestamp().nanos());
        }
        auto rightSampleOut = container->get(m_rightStreamId);
        if (rightSampleOut.get()) {
            auto rightCameraSampleOut = std::static_pointer_cast<filter_graph::AggregateSample<hal::CameraSample> >(rightSampleOut);
            EXPECT_EQ(m_times[m_counter].second, rightCameraSampleOut->data().systemtimestamp().nanos());
        }
        m_counter++;
    }

private:
    std::string m_leftStreamId;
    std::string m_rightStreamId;
    std::vector<std::pair<int64_t, int64_t> > m_times;
    uint32_t m_counter;
};

void transmitData(SyncMux& syncMux, std::string leftStreamId, std::string rightStreamId, std::vector<std::pair<int64_t, int64_t> > times) {

    for (uint32_t i = 0; i < times.size(); i++) {
        auto container = std::make_shared<filter_graph::Container>();
        auto leftSample = std::make_shared<filter_graph::AggregateSample<hal::CameraSample> >(leftStreamId);
        CameraSample& leftCameraSample = leftSample->data();
        auto rightSample = std::make_shared<filter_graph::AggregateSample<hal::CameraSample> >(rightStreamId);
        CameraSample& rightCameraSample = rightSample->data();

        SystemTimestamp* systemTimestamp = new SystemTimestamp();
        leftCameraSample.set_id(0);
        if (times[i].first != -1) {
            systemTimestamp->set_nanos(times[i].first);
            leftCameraSample.set_allocated_systemtimestamp(systemTimestamp);
            container->add(leftSample->streamId(), leftSample);
        }
        systemTimestamp = new SystemTimestamp();
        rightCameraSample.set_id(0);
        if (times[i].second != -1) {
            systemTimestamp->set_nanos(times[i].second);
            rightCameraSample.set_allocated_systemtimestamp(systemTimestamp);
            container->add(rightSample->streamId(), rightSample);
        }
        syncMux.receive(container);
    }
}
}

using namespace stereo_test;

TEST(StereoSyncMuxTest, canCreate) { EXPECT_NO_THROW(SyncMux syncMux("leftCam", "rightCam")); }

TEST(StereoSyncMuxTest, syncRightInitialDrop) {

    std::string leftStreamId = "leftCam";
    std::string rightStreamId = "rightCam";
    SyncMux syncMux(leftStreamId, rightStreamId);
    std::vector<std::pair<int64_t, int64_t> > inTimes;
    std::vector<std::pair<int64_t, int64_t> > outTimes;

    inTimes.push_back(std::make_pair<int64_t, int64_t>(1000000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(6000000, 7000000));

    outTimes.push_back(std::make_pair<int64_t, int64_t>(6000000, 7000000));
    auto testFilter = std::make_shared<TestFilter>(leftStreamId, rightStreamId, outTimes);

    syncMux.attachDownstreamFilter(testFilter);

    transmitData(syncMux, leftStreamId, rightStreamId, inTimes);
}

TEST(StereoSyncMuxTest, syncLeftInitialDrop) {

    std::string leftStreamId = "leftCam";
    std::string rightStreamId = "rightCam";
    SyncMux syncMux(leftStreamId, rightStreamId);
    std::vector<std::pair<int64_t, int64_t> > inTimes;
    std::vector<std::pair<int64_t, int64_t> > outTimes;

    inTimes.push_back(std::make_pair<int64_t, int64_t>(-1, 1000000));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(6000000, 7000000));

    outTimes.push_back(std::make_pair<int64_t, int64_t>(6000000, 7000000));
    auto testFilter = std::make_shared<TestFilter>(leftStreamId, rightStreamId, outTimes);

    syncMux.attachDownstreamFilter(testFilter);

    transmitData(syncMux, leftStreamId, rightStreamId, inTimes);
}

TEST(StereoSyncMuxTest, syncRightIntermediateDrop) {

    std::string leftStreamId = "leftCam";
    std::string rightStreamId = "rightCam";
    SyncMux syncMux(leftStreamId, rightStreamId);
    std::vector<std::pair<int64_t, int64_t> > inTimes;
    std::vector<std::pair<int64_t, int64_t> > outTimes;

    inTimes.push_back(std::make_pair<int64_t, int64_t>(10000000, 11000000));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(20000000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(30000004, 30000000));

    outTimes.push_back(std::make_pair<int64_t, int64_t>(10000000, 11000000));
    outTimes.push_back(std::make_pair<int64_t, int64_t>(30000004, 30000000));
    auto testFilter = std::make_shared<TestFilter>(leftStreamId, rightStreamId, outTimes);

    syncMux.attachDownstreamFilter(testFilter);

    transmitData(syncMux, leftStreamId, rightStreamId, inTimes);
}

TEST(StereoSyncMuxTest, syncNoMatchingData) {

    std::string leftStreamId = "leftCam";
    std::string rightStreamId = "rightCam";
    SyncMux syncMux(leftStreamId, rightStreamId);
    std::vector<std::pair<int64_t, int64_t> > inTimes;
    std::vector<std::pair<int64_t, int64_t> > outTimes;

    inTimes.push_back(std::make_pair<int64_t, int64_t>(-1, 11000000));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(20000000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(40000000, 30000000));

    auto testFilter = std::make_shared<TestFilter>(leftStreamId, rightStreamId, outTimes);

    syncMux.attachDownstreamFilter(testFilter);

    transmitData(syncMux, leftStreamId, rightStreamId, inTimes);
}

TEST(StereoSyncMuxTest, syncRightLongFrameDrop) {

    std::string leftStreamId = "leftCam";
    std::string rightStreamId = "rightCam";
    SyncMux syncMux(leftStreamId, rightStreamId);
    std::vector<std::pair<int64_t, int64_t> > inTimes;
    std::vector<std::pair<int64_t, int64_t> > outTimes;

    inTimes.push_back(std::make_pair<int64_t, int64_t>(10000000, 11000000));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(20000000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(30000000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(40000000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(50000000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(60005000, -1));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(70000000, 60000000));
    inTimes.push_back(std::make_pair<int64_t, int64_t>(80000000, 80000008));

    outTimes.push_back(std::make_pair<int64_t, int64_t>(10000000, 11000000));
    outTimes.push_back(std::make_pair<int64_t, int64_t>(60005000, 60000000));
    outTimes.push_back(std::make_pair<int64_t, int64_t>(80000000, 80000008));
    auto testFilter = std::make_shared<TestFilter>(leftStreamId, rightStreamId, outTimes);

    syncMux.attachDownstreamFilter(testFilter);

    transmitData(syncMux, leftStreamId, rightStreamId, inTimes);
}
