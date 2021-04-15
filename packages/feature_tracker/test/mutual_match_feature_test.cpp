#include "packages/feature_tracker/include/mutual_feature_match.h"

#include "gtest/gtest.h"

using namespace feature_tracker;

TEST(MutualMatchFeatureTest, updateMatch) {
    feature_tracker::mutual_feature_match mutualMatchFeature1;
    feature_tracker::mutual_feature_match mutualMatchFeature2;

    mutualMatchFeature1.updateBestMatch(&mutualMatchFeature2, 10.0f);
    EXPECT_FLOAT_EQ(10.0f, mutualMatchFeature1.bestScore);
    EXPECT_EQ(&mutualMatchFeature2, mutualMatchFeature1.bestMatch);
}

TEST(MutualMatchFeatureTest, updateMatchLowScore) {
    feature_tracker::mutual_feature_match mutualMatchFeature1;
    feature_tracker::mutual_feature_match mutualMatchFeature2;
    mutualMatchFeature1.bestScore = 100.0f;

    mutualMatchFeature1.updateBestMatch(&mutualMatchFeature2, 10.0f);
    EXPECT_FLOAT_EQ(100.0f, mutualMatchFeature1.bestScore);
    EXPECT_EQ(nullptr, mutualMatchFeature1.bestMatch);
}

TEST(MutualMatchFeatureTest, isMutuallyBestWithNoMatches) {
    feature_tracker::mutual_feature_match mutualMatchFeature1;
    EXPECT_FALSE(mutualMatchFeature1.isMutuallyBest());
}

TEST(MutualMatchFeatureTest, notMutuallyBest) {
    feature_tracker::mutual_feature_match mutualMatchFeature1;
    feature_tracker::mutual_feature_match mutualMatchFeature2;

    mutualMatchFeature1.updateBestMatch(&mutualMatchFeature2, 10.0f);
    EXPECT_FALSE(mutualMatchFeature1.isMutuallyBest());
    EXPECT_FALSE(mutualMatchFeature2.isMutuallyBest());
}

TEST(MutualMatchFeatureTest, isMutuallyBest) {
    feature_tracker::mutual_feature_match mutualMatchFeature1;
    feature_tracker::mutual_feature_match mutualMatchFeature2;

    mutualMatchFeature1.updateBestMatch(&mutualMatchFeature2, 10.0f);
    mutualMatchFeature2.updateBestMatch(&mutualMatchFeature1, 10.0f);
    EXPECT_TRUE(mutualMatchFeature1.isMutuallyBest());
    EXPECT_TRUE(mutualMatchFeature2.isMutuallyBest());
}
