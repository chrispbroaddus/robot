#pragma once

namespace feature_tracker {

/// @brief Match data-structure that keeps track of its best match and can check if it is a mutually best match
struct mutual_feature_match {

    /// Create MutualFeatureMatch structure and initialize its values
    mutual_feature_match() {
        bestScore = 0;
        bestMatch = nullptr;
    }

    /// Update the best match if the match score is higher
    void updateBestMatch(const mutual_feature_match* match, const float score) {
        if (score > bestScore) {
            bestMatch = match;
            bestScore = score;
        }
    }

    /// Check if the match is the mutually best match
    bool isMutuallyBest() const { return bestMatch && bestMatch->bestMatch == this; }

    /// Index of the corresponding feature in its feature store
    int featureIndex;
    /// Match score for the best match
    float bestScore;
    /// Pointer to the bestMatch. Non-owning pointer (Not used to allocate or deallocate new MutualMatchFeatures)
    /// The MutualMatchFeature should be in memory until the best matches and track indices are computed for all features
    const mutual_feature_match* bestMatch;
};
}
