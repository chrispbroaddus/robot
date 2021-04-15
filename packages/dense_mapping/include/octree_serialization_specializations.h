#pragma once

#include "octree.h"
#include "octree_payloads.h"
#include "packages/dense_mapping/proto/volumetric_time_series_dataset.pb.h"

namespace dense_mapping {
namespace serialization {
    namespace details {
        template <typename SCALAR_TYPE>
        struct SerializationHelper<SCALAR_TYPE, payloads::OccupiedOnlyOctreeLeafNode, dense_mapping::VolumetricModel> {
            static void serialize(const payloads::OccupiedOnlyOctreeLeafNode<SCALAR_TYPE>& node, dense_mapping::SpatialMoments* moments) {
                moments->set_centroid_x(node.m_centroidX);
                moments->set_centroid_y(node.m_centroidY);
                moments->set_centroid_z(node.m_centroidZ);
                moments->set_moment_xx(node.m_momentXX);
                moments->set_moment_xy(node.m_momentXY);
                moments->set_moment_xz(node.m_momentXZ);
                moments->set_moment_yy(node.m_momentYY);
                moments->set_moment_yz(node.m_momentYZ);
                moments->set_moment_zz(node.m_momentZZ);
            }

            static void serialize(dense_mapping::VolumetricModel& destination,
                const payloads::OccupiedOnlyOctreeLeafNode<SCALAR_TYPE>& leaf, const std::array<SCALAR_TYPE, 3>& voxelCenter,
                SCALAR_TYPE /* voxelHalfExtent */) {
                auto voxel = destination.add_voxels();
                voxel->set_voxel_center_x(voxelCenter[0]);
                voxel->set_voxel_center_y(voxelCenter[1]);
                voxel->set_voxel_center_z(voxelCenter[2]);
                voxel->set_occupancy_count(leaf.m_occupiedCount);
                serialize(leaf, voxel->mutable_spatial_moments());
            }

            template <template <typename> class INSERTION_POLICY>
            static void serialize(const Octree<SCALAR_TYPE, payloads::OccupiedOnlyOctreeLeafNode, INSERTION_POLICY>& tree,
                dense_mapping::VolumetricModel& destination) {
                destination.set_voxel_half_extent(tree.volumeLength() / (1LL << tree.leafDepth()));
                destination.set_volume_half_extent(tree.volumeLength());
            }
        };

        template <typename SCALAR_TYPE>
        struct SerializationHelper<SCALAR_TYPE, payloads::ExplicitFreeSpaceOctreeLeafNode, dense_mapping::VolumetricModel> {
            static void serialize(
                const payloads::ExplicitFreeSpaceOctreeLeafNode<SCALAR_TYPE>& node, dense_mapping::SpatialMoments* moments) {
                moments->set_centroid_x(node.m_centroidX);
                moments->set_centroid_y(node.m_centroidY);
                moments->set_centroid_z(node.m_centroidZ);
                moments->set_moment_xx(node.m_momentXX);
                moments->set_moment_xy(node.m_momentXY);
                moments->set_moment_xz(node.m_momentXZ);
                moments->set_moment_yy(node.m_momentYY);
                moments->set_moment_yz(node.m_momentYZ);
                moments->set_moment_zz(node.m_momentZZ);
            }

            static void serialize(
                const payloads::ExplicitFreeSpaceOctreeLeafNode<SCALAR_TYPE>& node, dense_mapping::EmptySpace* emptySpace) {
                emptySpace->set_empty_space_count(node.m_emptyCount);
            }

            static void serialize(const payloads::ExplicitFreeSpaceOctreeLeafNode<SCALAR_TYPE>& node,
                dense_mapping::AverageViewDirection* averageViewDirection) {
                averageViewDirection->set_x(node.m_averageViewDirectionX);
                averageViewDirection->set_y(node.m_averageViewDirectionY);
                averageViewDirection->set_z(node.m_averageViewDirectionZ);
            }

            static void serialize(dense_mapping::VolumetricModel& destination,
                const payloads::ExplicitFreeSpaceOctreeLeafNode<SCALAR_TYPE>& leaf, const std::array<SCALAR_TYPE, 3>& voxelCenter,
                SCALAR_TYPE /* voxelHalfExtent */) {
                auto voxel = destination.add_voxels();
                voxel->set_voxel_center_x(voxelCenter[0]);
                voxel->set_voxel_center_y(voxelCenter[1]);
                voxel->set_voxel_center_z(voxelCenter[2]);
                voxel->set_occupancy_count(leaf.m_occupiedCount);
                serialize(leaf, voxel->mutable_spatial_moments());
                serialize(leaf, voxel->mutable_empty_space());
                serialize(leaf, voxel->mutable_average_view_direction());
            }

            template <template <typename> class INSERTION_POLICY>
            static void serialize(const Octree<SCALAR_TYPE, payloads::ExplicitFreeSpaceOctreeLeafNode, INSERTION_POLICY>& tree,
                dense_mapping::VolumetricModel& destination) {
                destination.set_voxel_half_extent(tree.volumeLength() / (1LL << tree.leafDepth()));
                destination.set_volume_half_extent(tree.volumeLength());
            }
        };
    }
}
}