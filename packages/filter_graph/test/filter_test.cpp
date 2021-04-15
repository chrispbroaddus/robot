
#include "packages/filter_graph/include/sink_filter.h"
#include "packages/filter_graph/include/source_filter.h"
#include "packages/filter_graph/include/thread_pool.h"
#include "packages/filter_graph/include/transform_filter.h"
#include "gtest/gtest.h"

#include <thread>

using namespace filter_graph;

class MockSourceFilter : public SourceFilter {
public:
    MockSourceFilter(const std::string& filterName, const size_t outputQueueSize)
        : SourceFilter(filterName, outputQueueSize) {}
    ~MockSourceFilter() = default;

    virtual void create() {
        printf("MockSourceFilter::create\n");

        auto container = std::make_shared<Container>();

        getOutputQueue()->enqueue(container);
        send();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
};

class MockTransformFilter : public TransformFilter {
public:
    MockTransformFilter(const std::string& filterName, const size_t inputQueueSize, const size_t outputQueueSize)
        : TransformFilter(filterName, inputQueueSize, outputQueueSize) {}
    ~MockTransformFilter() = default;

    virtual void receive(std::shared_ptr<Container> container) {
        printf("MockTransformFilter::receive\n");

        getOutputQueue()->enqueue(container);
        send();
    }
};

class MockSinkFilter : public SinkFilter {
public:
    MockSinkFilter(const std::string& filterName, const size_t inputQueueSize)
        : SinkFilter(filterName, inputQueueSize) {}
    ~MockSinkFilter() = default;

    virtual void receive(std::shared_ptr<Container> container) { printf("MockSinkFilter::receive\n"); }
};

TEST(Filter, SimpleFilterGraph) {

    auto filterA = std::make_shared<MockSourceFilter>("sourceFilterA", 10);
    auto filterB = std::make_shared<MockTransformFilter>("filterB", 10, 10);
    auto filterC = std::make_shared<MockSinkFilter>("filterC", 10);

    filterA->attachDownstreamFilter(filterB);
    filterB->attachDownstreamFilter(filterC);

    ThreadPool<SourceFilterThreadRunner> sourceFilterThreadPool(1);
    ThreadPool<FilterThreadRunner> filterThreadPool(1);

    sourceFilterThreadPool.attachedFilter(filterA);

    filterThreadPool.attachedFilter(filterB);
    filterThreadPool.attachedFilter(filterC);

    filterThreadPool.initialize();
    sourceFilterThreadPool.initialize();

    filterA->attachThreadRunner(filterThreadPool.getThreadRunner(), false);
    filterB->attachThreadRunner(filterThreadPool.getThreadRunner(), false);
    filterC->attachThreadRunner(filterThreadPool.getThreadRunner(), false);

    sourceFilterThreadPool.start();
    filterThreadPool.start();

    std::this_thread::sleep_for(std::chrono::seconds(1));
}
