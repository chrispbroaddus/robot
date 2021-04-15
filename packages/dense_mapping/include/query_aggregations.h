#pragma once

#include <functional>
#include <limits>

#include "box_geometry.h"
#include "heightmap.h"

namespace dense_mapping {
namespace aggregations {

    template <typename OCTREE_TYPE> struct HeightMapAggregation {
        using octree_type = OCTREE_TYPE;
        using scalar_type = typename octree_type::scalar_type;
        using point_type = std::array<scalar_type, 3>;
        using leaf_node_type = typename octree_type::leaf_node_type;
        using heightmap_type = heightmap::HeightMap<scalar_type>;

        HeightMapAggregation(scalar_type heightMapExtentX, scalar_type heightMapExtentY, const point_type& center,
            const point_type& xDirection, const point_type& yDirection)
            : m_center(center)
            , m_xDirection(Geometry::unitVector(xDirection))
            , m_yDirection(Geometry::unitVector(yDirection))
            , m_zDirection(Geometry::crossProduct(m_xDirection, m_yDirection))
            , m_heightMap(center[0], center[1], heightMapExtentX, heightMapExtentY)
            , m_visitor([&](const leaf_node_type&, const point_type& voxelCenter, scalar_type voxelHalfExtent) {
                const point_type center{ { voxelCenter[0] - m_center[0], voxelCenter[1] - m_center[1], voxelCenter[2] - m_center[2] } };
                const scalar_type projectionX = Geometry::innerProduct(center, m_xDirection);
                const scalar_type projectionY = Geometry::innerProduct(center, m_yDirection);

                scalar_type projectionZ = -std::numeric_limits<scalar_type>::infinity();

                for (int corner = 0; corner < 8; ++corner) {
                    const point_type c{ { center[0] + ((corner & 1) ? voxelHalfExtent : -voxelHalfExtent),
                        center[1] + ((corner & 2) ? voxelHalfExtent : -voxelHalfExtent),
                        center[2] + ((corner & 4) ? voxelHalfExtent : -voxelHalfExtent) } };

                    projectionZ = std::max(projectionZ, Geometry::innerProduct(c, m_zDirection));
                }

                m_heightMap.insert(projectionX, projectionY, projectionZ);
            }) {

            if (std::abs(Geometry::innerProduct(m_xDirection, m_yDirection)) > std::numeric_limits<scalar_type>::epsilon()) {
                throw std::runtime_error("Invalid X and Y direction -- these should be orthogonal");
            }
        }

        const std::array<scalar_type, 3> m_center;
        const std::array<scalar_type, 3> m_xDirection;
        const std::array<scalar_type, 3> m_yDirection;
        const std::array<scalar_type, 3> m_zDirection;
        heightmap_type m_heightMap;

        std::function<void(const leaf_node_type&, const point_type&, scalar_type)> m_visitor;
    };

    /// Find the closest voxel to a ray. Specifically:
    /// For each voxel that is visited, project its center onto the ray. If the center projects into the positive
    /// half-space and the projection is smaller than any other previously visited voxels, remember this one as the new
    /// closest voxel. If no such voxel is found, m_nodeProjection will be std::numeric_limits<scalar_type>::infinity,
    /// otherwise a copy of the voxel will be in m_node.
    ///
    /// \tparam OCTREE_TYPE
    template <typename OCTREE_TYPE> struct NearestPointOnRayAggregation {
        using octree_type = OCTREE_TYPE;
        using scalar_type = typename octree_type::scalar_type;
        using point_type = typename octree_type::point_type;
        using leaf_node_type = typename octree_type::leaf_node_type;

        /// Capture details of the ray onto which we project the centers of visited voxels.
        /// \param rayOrigin
        /// \param rayDirection
        NearestPointOnRayAggregation(const point_type& rayOrigin, const point_type& rayDirection)
            : m_point(rayOrigin)
            , m_direction(rayDirection)
            , m_nodeProjection(std::numeric_limits<scalar_type>::infinity())
            , m_node()
            , m_visitor([&](const leaf_node_type& voxel, const point_type& voxelCenter, scalar_type voxelHalfExtent) {
                const point_type center = Geometry::directionVector(m_point, voxelCenter);
                const scalar_type product = Geometry::innerProduct(center, m_direction);

                if (product >= 0 && product < m_nodeProjection) {
                    m_nodeProjection = product;
                    m_node = voxel;
                }
            }) {}

        const std::array<scalar_type, 3> m_point;
        const std::array<scalar_type, 3> m_direction;

        scalar_type m_nodeProjection;
        leaf_node_type m_node;

        std::function<void(const leaf_node_type&, const point_type&, scalar_type)> m_visitor;
    };
}
}