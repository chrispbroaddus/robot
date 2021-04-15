
#include "packages/filter_graph/include/container.h"
#include "gtest/gtest.h"

using namespace filter_graph;

TEST(Container, addGetSample) {

    Container container;

    container.add("myStreamId", std::shared_ptr<Sample>(new Sample("myStreamId")));
    std::shared_ptr<Sample> sample = container.get("myStreamId");

    EXPECT_EQ("myStreamId", sample->streamId());
}

TEST(Container, addGetConstSample) {

    const Container container("myStreamId", std::shared_ptr<Sample>(new Sample("myStreamId")));

    std::shared_ptr<const Sample> sample = container.get("myStreamId");

    EXPECT_EQ("myStreamId", sample->streamId());
}

TEST(Container, eraseSample) {

    Container container;

    container.add("myStreamId", std::shared_ptr<Sample>(new Sample("myStreamId")));

    EXPECT_EQ("myStreamId", container.get("myStreamId")->streamId());

    container.erase("myStreamId");

    EXPECT_EQ(0, container.get("myStreamId").get());
}
