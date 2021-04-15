#include "packages/feature_tracker/include/ncc_feature_matcher.h"

#include "gtest/gtest.h"

using namespace feature_tracker;

TEST(FeatureMatcherTest, badImageSizeRows) {
    EXPECT_THROW(feature_tracker::NccFeatureMatcher<11> featureMatcher(100, 0, 2448, -1), std::runtime_error);
}

TEST(FeatureMatcherTest, badImageSizeCols) {
    EXPECT_THROW(feature_tracker::NccFeatureMatcher<11> featureMatcher(100, 2048, 0, -1), std::runtime_error);
}

TEST(FeatureMatcherTest, badImageSearchRadius) {
    EXPECT_THROW(feature_tracker::NccFeatureMatcher<11> featureMatcher(0, 100, 100, -1), std::runtime_error);
}

TEST(FeatureMatcherTest, canInstantiate) {
    feature_tracker::NccFeatureMatcher<11> featureMatcher(150, 2048, 2448, -1);
    EXPECT_EQ(300, featureMatcher.getGridSize());
    EXPECT_EQ(7, featureMatcher.getNumGridRows());
    EXPECT_EQ(9, featureMatcher.getNumGridCols());
}

TEST(FeatureMatcherTest, singleFrameMatching) {

    constexpr size_t windowSize = 3;
    constexpr uint32_t imageRows = 480;
    constexpr uint32_t imageCols = 640;
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector;
    feature_tracker::NccFeature<windowSize> feature1;
    constexpr uint8_t imagePatch1[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    feature1.A = 1;
    feature1.B = 2;
    feature1.C = 1.1;
    feature1.x = 2.1;
    feature1.y = 3.1;
    feature1.featurePointIndex = 3;
    memcpy(feature1.imagePatch, imagePatch1, 9);
    nccFeatureVector.push_back(feature1);
    feature_tracker::NccFeature<windowSize> feature2;
    constexpr uint8_t imagePatch2[9] = { 3, 1, 2, 4, 3, 2, 8, 7, 2 };
    feature2.A = 3;
    feature2.B = 4;
    feature2.C = 1.2;
    feature2.x = 2.2;
    feature2.y = 3.2;
    feature2.featurePointIndex = 7;
    memcpy(feature2.imagePatch, imagePatch2, 9);
    nccFeatureVector.push_back(feature2);
    feature_tracker::NccFeatureStore<windowSize> featureStore(nccFeatureVector, imageRows, imageCols);

    feature_tracker::NccFeatureMatcher<windowSize> featureMatcher(100, featureStore.getImageRows(), featureStore.getImageCols(), -1);

    featureMatcher.updateCurrentFrame(&featureStore);
    std::vector<std::pair<int, int> > matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(0, matchedIndices.size());
}

TEST(FeatureMatcherTest, noFeatureChangeMatching) {

    constexpr size_t windowSize = 3;
    constexpr uint32_t imageRows = 480;
    constexpr uint32_t imageCols = 640;
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector;
    feature_tracker::NccFeature<windowSize> feature1;
    constexpr uint8_t imagePatch1[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    feature1.A = 45;
    feature1.B = 285;
    feature1.C = 0.043033148f;
    feature1.x = 2.1;
    feature1.y = 3.1;
    feature1.featurePointIndex = 3;
    memcpy(feature1.imagePatch, imagePatch1, 9);
    nccFeatureVector.push_back(feature1);
    feature_tracker::NccFeature<windowSize> feature2;
    constexpr uint8_t imagePatch2[9] = { 3, 1, 2, 4, 3, 2, 8, 7, 2 };
    feature2.A = 32;
    feature2.B = 160;
    feature2.C = 0.049029034f;
    feature2.x = 380.2;
    feature2.y = 400.2;
    feature2.featurePointIndex = 7;
    memcpy(feature2.imagePatch, imagePatch2, 9);
    nccFeatureVector.push_back(feature2);
    feature_tracker::NccFeatureStore<windowSize> featureStore1(nccFeatureVector, imageRows, imageCols);
    feature_tracker::NccFeatureStore<windowSize> featureStore2(nccFeatureVector, imageRows, imageCols);

    feature_tracker::NccFeatureMatcher<windowSize> featureMatcher(50, featureStore1.getImageRows(), featureStore1.getImageCols(), -1);

    featureMatcher.updateCurrentFrame(&featureStore1);
    featureMatcher.updateCurrentFrame(&featureStore2);
    std::vector<std::pair<int, int> > matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(2, matchedIndices.size());
    EXPECT_EQ(3, matchedIndices[0].first);
    EXPECT_EQ(3, matchedIndices[0].second);
    EXPECT_EQ(7, matchedIndices[1].first);
    EXPECT_EQ(7, matchedIndices[1].second);
}

TEST(FeatureMatcherTest, multipleCandidateMatching) {

    constexpr size_t windowSize = 3;
    constexpr uint32_t imageRows = 480;
    constexpr uint32_t imageCols = 640;
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector1;
    feature_tracker::NccFeature<windowSize> feature11;
    constexpr uint8_t imagePatch11[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    feature11.A = 45;
    feature11.B = 285;
    feature11.C = 0.043033148f;
    feature11.x = 200.1;
    feature11.y = 300.1;
    feature11.featurePointIndex = 5;
    memcpy(feature11.imagePatch, imagePatch11, 9);
    nccFeatureVector1.push_back(feature11);
    feature_tracker::NccFeature<windowSize> feature12;
    constexpr uint8_t imagePatch12[9] = { 3, 1, 2, 4, 3, 2, 8, 7, 2 };
    feature12.A = 32;
    feature12.B = 160;
    feature12.C = 0.049029034f;
    feature12.x = 380.2;
    feature12.y = 400.2;
    feature12.featurePointIndex = 7;
    memcpy(feature12.imagePatch, imagePatch12, 9);
    nccFeatureVector1.push_back(feature12);
    feature_tracker::NccFeatureStore<windowSize> featureStore1(nccFeatureVector1, imageRows, imageCols);

    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector2;
    feature_tracker::NccFeature<windowSize> feature21;
    constexpr uint8_t imagePatch1[9] = { 3, 3, 2, 4, 3, 2, 8, 7, 2 };
    feature21.A = 34;
    feature21.B = 168;
    feature21.C = 0.052999894f;
    feature21.x = 408.1;
    feature21.y = 365.1;
    feature21.featurePointIndex = 2;
    memcpy(feature21.imagePatch, imagePatch1, 9);
    nccFeatureVector2.push_back(feature21);
    feature_tracker::NccFeature<windowSize> feature22;
    constexpr uint8_t imagePatch2[9] = { 1, 3, 2, 5, 3, 2, 2, 7, 2 };
    feature22.A = 27;
    feature22.B = 109;
    feature22.C = 0.062994079f;
    feature22.x = 360.2;
    feature22.y = 410.2;
    feature22.featurePointIndex = 6;
    memcpy(feature22.imagePatch, imagePatch2, 9);
    nccFeatureVector2.push_back(feature22);
    feature_tracker::NccFeatureStore<windowSize> featureStore2(nccFeatureVector2, imageRows, imageCols);

    feature_tracker::NccFeatureMatcher<windowSize> featureMatcher(50, featureStore1.getImageRows(), featureStore1.getImageCols(), -1);

    featureMatcher.updateCurrentFrame(&featureStore1);
    featureMatcher.updateCurrentFrame(&featureStore2);
    std::vector<std::pair<int, int> > matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(1, matchedIndices.size());
    EXPECT_EQ(7, matchedIndices[0].first);
    EXPECT_EQ(2, matchedIndices[0].second);
}

TEST(FeatureMatcherTest, multipleCandidateMatchingMultiFrame) {

    constexpr size_t windowSize = 3;
    constexpr uint32_t imageRows = 480;
    constexpr uint32_t imageCols = 640;
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector1;
    feature_tracker::NccFeature<windowSize> feature11;
    constexpr uint8_t imagePatch11[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    feature11.A = 45;
    feature11.B = 285;
    feature11.C = 0.043033148f;
    feature11.x = 200.1;
    feature11.y = 300.1;
    feature11.featurePointIndex = 5;
    memcpy(feature11.imagePatch, imagePatch11, 9);
    nccFeatureVector1.push_back(feature11);
    feature_tracker::NccFeature<windowSize> feature12;
    constexpr uint8_t imagePatch12[9] = { 3, 1, 2, 4, 3, 2, 8, 7, 2 };
    feature12.A = 32;
    feature12.B = 160;
    feature12.C = 0.049029034f;
    feature12.x = 380.2;
    feature12.y = 400.2;
    feature12.featurePointIndex = 7;
    memcpy(feature12.imagePatch, imagePatch12, 9);
    nccFeatureVector1.push_back(feature12);
    feature_tracker::NccFeatureStore<windowSize> featureStore1(nccFeatureVector1, imageRows, imageCols);

    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector2;
    feature_tracker::NccFeature<windowSize> feature21;
    constexpr uint8_t imagePatch1[9] = { 3, 3, 2, 4, 3, 2, 8, 7, 2 };
    feature21.A = 34;
    feature21.B = 168;
    feature21.C = 0.052999894f;
    feature21.x = 408.1;
    feature21.y = 365.1;
    feature21.featurePointIndex = 2;
    memcpy(feature21.imagePatch, imagePatch1, 9);
    nccFeatureVector2.push_back(feature21);
    feature_tracker::NccFeature<windowSize> feature22;
    constexpr uint8_t imagePatch2[9] = { 1, 3, 2, 5, 3, 2, 2, 7, 2 };
    feature22.A = 27;
    feature22.B = 109;
    feature22.C = 0.062994079f;
    feature22.x = 360.2;
    feature22.y = 410.2;
    feature22.featurePointIndex = 6;
    memcpy(feature22.imagePatch, imagePatch2, 9);
    nccFeatureVector2.push_back(feature22);
    feature_tracker::NccFeatureStore<windowSize> featureStore2(nccFeatureVector2, imageRows, imageCols);
    feature_tracker::NccFeatureStore<windowSize> featureStore3(nccFeatureVector1, imageRows, imageCols);

    feature_tracker::NccFeatureMatcher<windowSize> featureMatcher(50, featureStore1.getImageRows(), featureStore1.getImageCols(), -1);

    featureMatcher.updateCurrentFrame(&featureStore1);
    featureMatcher.updateCurrentFrame(&featureStore2);
    std::vector<std::pair<int, int> > matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(1, matchedIndices.size());
    EXPECT_EQ(7, matchedIndices[0].first);
    EXPECT_EQ(2, matchedIndices[0].second);

    featureMatcher.updateCurrentFrame(&featureStore3);
    matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(1, matchedIndices.size());
    EXPECT_EQ(2, matchedIndices[0].first);
    EXPECT_EQ(7, matchedIndices[0].second);
}
