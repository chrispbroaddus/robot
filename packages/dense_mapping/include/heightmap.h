#pragma once

#include "packages/benchmarking/include/summary_statistics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace dense_mapping {
namespace heightmap {
    namespace details {
        enum struct ChildAddress : int32_t { Invalid = -1, NorthWest, NorthEast, SouthWest, SouthEast };

        template <typename SCALAR_TYPE>
        constexpr bool containedInNorthWestChild(
            SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE centerX, SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY) {
            const auto dx = x - centerX;
            const auto dy = y - centerY;
            const auto xOverlaps = (dx >= -extentX) && (dx <= 0);
            const auto yOverlaps = (dy >= -extentY) && (dy <= 0);
            return xOverlaps && yOverlaps;
        }

        template <typename SCALAR_TYPE>
        constexpr bool containedInNorthEastChild(
            SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE centerX, SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY) {
            const auto dx = x - centerX;
            const auto dy = y - centerY;
            const auto xOverlaps = (dx >= 0) && (dx <= extentX);
            const auto yOverlaps = (dy >= -extentY) && (dy <= 0);
            return xOverlaps && yOverlaps;
        }

        template <typename SCALAR_TYPE>
        constexpr bool containedInSouthWestChild(
            SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE centerX, SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY) {
            const auto dx = x - centerX;
            const auto dy = y - centerY;
            const auto xOverlaps = (dx >= -extentX) && (dx <= 0);
            const auto yOverlaps = (dy >= 0) && (dy <= extentY);
            return xOverlaps && yOverlaps;
        }

        template <typename SCALAR_TYPE>
        constexpr bool containedInSouthEastChild(
            SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE centerX, SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY) {
            const auto dx = x - centerX;
            const auto dy = y - centerY;
            const auto xOverlaps = (dx >= 0) && (dx <= extentX);
            const auto yOverlaps = (dy >= 0) && (dy <= extentY);
            return xOverlaps && yOverlaps;
        }

        template <typename SCALAR_TYPE>
        constexpr ChildAddress getChild(
            SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE centerX, SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY) {

            const auto xInRange = std::abs(x - centerX) <= extentX;
            const auto yInRange = std::abs(y - centerY) <= extentY;

            if (xInRange && yInRange) {
                if (containedInNorthEastChild(x, y, centerX, centerY, extentX, extentY)) {
                    return ChildAddress::NorthEast;
                } else if (containedInNorthWestChild(x, y, centerX, centerY, extentX, extentY)) {
                    return ChildAddress::NorthWest;
                } else if (containedInSouthEastChild(x, y, centerX, centerY, extentX, extentY)) {
                    return ChildAddress::SouthEast;
                } else {
                    return ChildAddress::SouthWest;
                }
            } else {
                return ChildAddress::Invalid;
            }
        }

        template <typename SCALAR_TYPE> constexpr SCALAR_TYPE childCenterX(ChildAddress address, SCALAR_TYPE centerX, SCALAR_TYPE extentX) {
            if (ChildAddress::Invalid == address) {
                throw std::runtime_error("Cannot compute child center X when address is ChildAddress::Invalid");
            }

            return centerX + ((ChildAddress::NorthWest == address || ChildAddress::SouthWest == address) ? -1 : 1) * extentX / 2;
        }

        template <typename SCALAR_TYPE> constexpr SCALAR_TYPE childCenterY(ChildAddress address, SCALAR_TYPE centerY, SCALAR_TYPE extentY) {
            if (ChildAddress::Invalid == address) {
                throw std::runtime_error("Cannot compute child center Y when address is ChildAddress::Invalid");
            }

            return centerY + ((ChildAddress::NorthEast == address || ChildAddress::NorthWest == address) ? -1 : 1) * extentY / 2;
        }
    }

    template <typename SCALAR_TYPE> class HeightMap {
    public:
        constexpr HeightMap(SCALAR_TYPE centerX, SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY)
            : m_centerX(centerX)
            , m_centerY(centerY)
            , m_halfExtentX(extentX)
            , m_halfExtentY(extentY)
            , m_root() {

            if (!std::isfinite(m_centerX)) {
                throw std::runtime_error("Invalid x center: must be finite");
            }

            if (!std::isfinite(m_centerY)) {
                throw std::runtime_error("Invalid y center: must be finite");
            }

            if (!std::isfinite(m_halfExtentX)) {
                throw std::runtime_error("Invalid x extent: must be finite");
            }

            if (m_halfExtentX <= 0) {
                throw std::runtime_error("Invalid x extent: must be strictly positive");
            }

            if (!std::isfinite(m_halfExtentY)) {
                throw std::runtime_error("Invalid y extent: must be finite");
            }

            if (m_halfExtentY <= 0) {
                throw std::runtime_error("Invalid y extent: must be strictly positive");
            }
        }

        void insert(SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE z) {
            insert(m_centerX, m_centerY, m_halfExtentX, m_halfExtentY, x, y, z, m_root);
        }

        void prunePoints() { prunePoints(m_root); }

        constexpr std::pair<size_t, SCALAR_TYPE> estimateHeightAtLocation(SCALAR_TYPE x, SCALAR_TYPE y) const {
            return estimateHeightAtLocation(x, y, m_centerX, m_centerY, m_halfExtentX, m_halfExtentY, m_root);
        }

        constexpr size_t cellCount() const { return cellCount(m_root); }

        void serialize(std::ostream& out) const {
            traverse(m_root, [&out](const Node& n) {
                out << "CENTER: [" << n.m_x << ", " << n.m_y << "] EXTENTS: [" << n.m_extentX << ", " << n.m_extentY << "], COUNT(TOTAL): ["
                    << n.m_zStatistics.count() << "], COUNT(DATA): [" << n.m_data.size() << "], VARIANCE: [" << n.m_zStatistics.variance()
                    << "], MAX: [" << n.m_zStatistics.maximum() << "]" << std::endl;
            });
        }

    private:
        /// Controls splitting -- need at least this many points to consider splitting a node
        static constexpr size_t kPointCountSplittingThreshold = 10;

        /// Controls splitting -- need variance at least this much in Z to consider splitting a node
        static constexpr SCALAR_TYPE kZVarianceSplittingThreshold = SCALAR_TYPE(1) / 100000000LL;

        /// Controls splitting -- split cells need to be at least this big on each side
        static constexpr SCALAR_TYPE kMinimumCellExtentFraction = SCALAR_TYPE(1) / 100;

        struct Node {
            Node(SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE extentX, SCALAR_TYPE extentY)
                : m_x(x)
                , m_y(y)
                , m_extentX(extentX)
                , m_extentY(extentY)
                , m_zStatistics()
                , m_isLeaf(true)
                , m_children()
                , m_data() {

                if (!std::isfinite(m_x)) {
                    throw std::runtime_error("Invalid x center: must be finite");
                }

                if (!std::isfinite(m_y)) {
                    throw std::runtime_error("Invalid y center: must be finite");
                }

                if (!std::isfinite(m_extentX)) {
                    throw std::runtime_error("Invalid x extent: must be finite");
                }

                if (m_extentX <= 0) {
                    throw std::runtime_error("Invalid x extent: must be strictly positive");
                }

                if (!std::isfinite(m_extentY)) {
                    throw std::runtime_error("Invalid y extent: must be finite");
                }

                if (m_extentY <= 0) {
                    throw std::runtime_error("Invalid y extent: must be strictly positive");
                }
            }

            SCALAR_TYPE m_x;
            SCALAR_TYPE m_y;
            SCALAR_TYPE m_extentX;
            SCALAR_TYPE m_extentY;
            SummaryStatistics<SCALAR_TYPE> m_zStatistics;

            bool m_isLeaf;
            std::array<std::shared_ptr<Node>, 4> m_children;
            std::vector<std::array<SCALAR_TYPE, 3> > m_data;
        };

        SCALAR_TYPE m_centerX;
        SCALAR_TYPE m_centerY;
        SCALAR_TYPE m_halfExtentX;
        SCALAR_TYPE m_halfExtentY;
        std::shared_ptr<Node> m_root;

        constexpr std::pair<size_t, SCALAR_TYPE> estimateHeightAtLocation(SCALAR_TYPE x, SCALAR_TYPE y, SCALAR_TYPE centerX,
            SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY, const std::shared_ptr<Node>& root) const {
            const auto child = details::getChild(x, y, centerX, centerY, extentX, extentY);

            if (!root || child == details::ChildAddress::Invalid) {
                return std::pair<size_t, SCALAR_TYPE>(0, std::numeric_limits<SCALAR_TYPE>::infinity());
            } else if (root->m_isLeaf) {
                return std::make_pair(root->m_zStatistics.count(), root->m_zStatistics.maximum());
            } else {
                const auto childHalfExtentX = extentX / 2;
                const auto childHalfExtentY = extentY / 2;
                const auto childX = childCenterX(child, centerX, extentX);
                const auto childY = childCenterY(child, centerY, extentY);
                return estimateHeightAtLocation(
                    x, y, childX, childY, childHalfExtentX, childHalfExtentY, root->m_children.at(static_cast<size_t>(child)));
            }
        }

        constexpr bool shouldSplit(const std::shared_ptr<Node>& root) const {
            return (root->m_data.size() >= kPointCountSplittingThreshold)
                && (root->m_zStatistics.variance() >= kZVarianceSplittingThreshold)
                && ((root->m_extentX / 2) >= kMinimumCellExtentFraction * m_halfExtentX)
                && ((root->m_extentY / 2) >= kMinimumCellExtentFraction * m_halfExtentY);
        }

        void split(std::shared_ptr<Node>& root) {
            std::vector<std::array<SCALAR_TYPE, 3> > points;
            root->m_isLeaf = false;
            std::swap(root->m_data, points);

            for (const auto& p : points) {
                const auto child = details::getChild(p[0], p[1], root->m_x, root->m_y, root->m_extentX, root->m_extentY);
                const auto childX = details::childCenterX(child, root->m_x, root->m_extentX);
                const auto childY = details::childCenterY(child, root->m_y, root->m_extentY);
                insert(childX, childY, root->m_extentX / 2, root->m_extentY / 2, p[0], p[1], p[2],
                    root->m_children.at(static_cast<size_t>(child)));
            }
        }

        void insert(SCALAR_TYPE centerX, SCALAR_TYPE centerY, SCALAR_TYPE extentX, SCALAR_TYPE extentY, SCALAR_TYPE x, SCALAR_TYPE y,
            SCALAR_TYPE z, std::shared_ptr<Node>& root) {

            const auto overlapsInX = std::abs(x - m_centerX) <= m_halfExtentX;
            const auto overlapsInY = std::abs(y - m_centerY) <= m_halfExtentY;

            if (overlapsInX && overlapsInY) {
                if (!root) {
                    root = std::make_shared<Node>(centerX, centerY, extentX, extentY);
                }

                root->m_zStatistics.update(z);

                if (root->m_isLeaf) {
                    root->m_data.push_back(std::array<SCALAR_TYPE, 3>{ { x, y, z } });

                    if (shouldSplit(root)) {
                        split(root);
                    }

                } else {
                    const auto child = details::getChild(x, y, centerX, centerY, extentX, extentY);
                    const auto childX = details::childCenterX(child, centerX, extentX);
                    const auto childY = details::childCenterY(child, centerY, extentY);
                    insert(childX, childY, extentX / 2, extentY / 2, x, y, z, root->m_children.at(static_cast<size_t>(child)));
                }
            }
        }

        void traverse(const std::shared_ptr<Node>& root, std::function<void(const Node&)> visitor) const {
            if (root) {
                visitor(*root);

                for (const auto& c : root->m_children) {
                    traverse(c, visitor);
                }
            }
        }

        void traverse(std::shared_ptr<Node>& root, std::function<void(Node&)> visitor) {
            if (root) {
                visitor(*root);

                for (const auto& c : root->m_children) {
                    traverse(c, visitor);
                }
            }
        }

        void prunePoints(std::shared_ptr<Node>& root) {
            if (root) {
                root->m_data.clear();

                for (auto& child : root->m_children) {
                    prunePoints(child);
                }
            }
        }

        constexpr size_t cellCount(const std::shared_ptr<Node>& root) const {
            if (root) {
                size_t count = 1; // 1 for me
                for (const auto& child : root->m_children) {
                    count += cellCount(child);
                }

                return count;
            } else {
                return 0;
            }
        }
    };

    template <typename T> constexpr size_t HeightMap<T>::kPointCountSplittingThreshold;

    template <typename T> constexpr T HeightMap<T>::kZVarianceSplittingThreshold;

    template <typename T> constexpr T HeightMap<T>::kMinimumCellExtentFraction;
}
}