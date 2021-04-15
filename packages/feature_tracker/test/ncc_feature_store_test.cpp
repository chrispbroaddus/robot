#include "packages/feature_tracker/include/ncc_feature_store.h"

#include "gtest/gtest.h"

TEST(NccFeatureStoreTest, VerifyComputeNccCoefficients) {

    constexpr unsigned char image[] = { 1, 7, 2, 3, 10, 4, 1, 7, 3, 10, 7, 6, 4, 5, 10, 8, 6, 4, 3, 9, 1, 10, 9, 1, 1, 7, 9, 2, 7, 9, 1, 7,
        9, 10, 8, 6, 7, 5, 3, 3, 6, 9, 1, 2, 3, 7, 2, 5, 9, 6, 1, 5, 4, 2, 5, 7, 4, 2, 10, 1, 9, 5, 6, 4, 6, 2, 5, 6, 8, 5, 9, 9, 5, 2, 10,
        2, 10, 3, 4, 4, 8, 1, 7, 5, 5, 10, 2, 4, 6, 2, 2, 2, 7, 4, 10, 2, 9, 6, 2, 2 };

    core::ImageView<core::ImageType::uint8> imageView(10, 10, 10, (unsigned char*)image);

    feature_detectors::FeaturePoint featurePoint1;
    feature_detectors::FeaturePoint featurePoint2;
    feature_detectors::FeaturePoint featurePoint3;
    feature_detectors::FeaturePoint featurePoint4;
    std::vector<feature_detectors::FeaturePoint> featurePoints;
    featurePoint1.set_x(3.1);
    featurePoint1.set_y(5.2);
    featurePoints.push_back(featurePoint1);
    constexpr unsigned char imagePatch1[] = { 1, 2, 3, 4, 2, 5, 6, 4, 6 };
    featurePoint2.set_x(0.5);
    featurePoint2.set_y(1);
    featurePoints.push_back(featurePoint2);
    featurePoint3.set_x(5.2);
    featurePoint3.set_y(9.1);
    featurePoints.push_back(featurePoint3);
    featurePoint4.set_x(8.6);
    featurePoint4.set_y(8.1);
    featurePoints.push_back(featurePoint4);
    constexpr unsigned char imagePatch4[] = { 3, 4, 4, 4, 6, 2, 6, 2, 2 };

    feature_tracker::NccFeatureStore<3> featureStore(imageView, featurePoints);

    EXPECT_EQ(2, featureStore.getStoreSize());
    EXPECT_EQ(0, featureStore.getFeaturePointIndex(0));
    EXPECT_EQ(33, featureStore.getA(0));
    EXPECT_EQ(147, featureStore.getB(0));
    EXPECT_FLOAT_EQ(0.065372045, featureStore.getC(0));
    for (int ctr = 0; ctr < 9; ctr++) {
        EXPECT_EQ(*(imagePatch1 + ctr), *(featureStore.getImagePatch(0) + ctr));
    }
    EXPECT_EQ(3, featureStore.getFeaturePointIndex(1));
    EXPECT_EQ(33, featureStore.getA(1));
    EXPECT_EQ(141, featureStore.getB(1));
    EXPECT_FLOAT_EQ(0.074535599, featureStore.getC(1));
    for (int ctr = 0; ctr < 9; ctr++) {
        EXPECT_EQ(*(imagePatch4 + ctr), *(featureStore.getImagePatch(1) + ctr));
    }
}

TEST(NccFeatureStoreTest, createFromNccFeatureVector) {

    constexpr size_t windowSize = 3;
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector;
    feature_tracker::NccFeature<windowSize> feature1;
    constexpr uint8_t imagePatch1[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    feature1.A = 1;
    feature1.B = 2;
    feature1.C = 1.1;
    feature1.x = 2.1;
    feature1.y = 3.1;
    feature1.featurePointIndex = 2;
    memcpy(feature1.imagePatch, imagePatch1, 9);
    nccFeatureVector.push_back(feature1);
    feature_tracker::NccFeature<windowSize> feature2;
    constexpr uint8_t imagePatch2[9] = { 3, 1, 2, 4, 3, 2, 8, 7, 2 };
    feature2.A = 3;
    feature2.B = 4;
    feature2.C = 1.2;
    feature2.x = 2.2;
    feature2.y = 3.2;
    feature2.featurePointIndex = 5;
    memcpy(feature2.imagePatch, imagePatch2, 9);
    nccFeatureVector.push_back(feature2);

    uint32_t imageRows = 100;
    uint32_t imageCols = 100;
    feature_tracker::NccFeatureStore<windowSize> featureStore(nccFeatureVector, imageRows, imageCols);

    EXPECT_EQ(2, featureStore.getStoreSize());
    EXPECT_EQ(2, featureStore.getFeaturePointIndex(0));
    EXPECT_EQ(1, featureStore.getA(0));
    EXPECT_EQ(2, featureStore.getB(0));
    EXPECT_FLOAT_EQ(1.1, featureStore.getC(0));
    EXPECT_FLOAT_EQ(2.1, featureStore.getX(0));
    EXPECT_FLOAT_EQ(3.1, featureStore.getY(0));
    for (int ctr = 0; ctr < 9; ctr++) {
        EXPECT_EQ(*(imagePatch1 + ctr), *(featureStore.getImagePatch(0) + ctr));
    }
    EXPECT_EQ(5, featureStore.getFeaturePointIndex(1));
    EXPECT_EQ(3, featureStore.getA(1));
    EXPECT_EQ(4, featureStore.getB(1));
    EXPECT_FLOAT_EQ(1.2, featureStore.getC(1));
    EXPECT_FLOAT_EQ(2.2, featureStore.getX(1));
    EXPECT_FLOAT_EQ(3.2, featureStore.getY(1));
    for (int ctr = 0; ctr < 9; ctr++) {
        EXPECT_EQ(*(imagePatch2 + ctr), *(featureStore.getImagePatch(1) + ctr));
    }
}
