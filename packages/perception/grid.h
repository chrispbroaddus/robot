#pragma once

#include "packages/perception/proto/grid.pb.h"
#include "packages/perception/proto/grid_options.pb.h"
#include "packages/perception/types.h"

namespace perception {

constexpr int kMinObservationsPerSurflet = 10;
constexpr int kMaxObservationsPerVoxel = 20;

/**
 * @brief The underlying voxel type. This records:
 *        n: the number of observations
 *        valid: whether there are sufficient observations
 *        centre: the position of the voxel in the grid
 *        variance: the surflet uncertainty
 *        normal: the surflet direction
 *        observations: the point observations associated wih this voxel
 */
typedef struct {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    int n;
    bool valid;
    Eigen::Vector3f centre;
    Eigen::Vector3f mean;
    Eigen::Matrix3f variance;
    Eigen::Vector3f normal;
    Eigen::Matrix<float, kMaxObservationsPerVoxel, 3> observations;
} Voxel;

/**
 * @brief A simple representation of a dense voxel grid.
 */
class VoxelGrid {
public:
    VoxelGrid(const VoxelGridOptions& options);

    ~VoxelGrid();

    bool add(const perception::PointCloudXYZ& cloud);

    void clear();

    const Voxel* operator()(int i, int j, int k) const;

    const Voxel* pointToVoxel(const Eigen::Vector3f& point) const;

    const std::vector<Voxel*>& voxels() const { return m_voxels; }

    void computeSurflets();

    VoxelGridOptions options() const { return m_options; }

private:
    VoxelGrid(const VoxelGrid& grid) = delete;
    VoxelGrid& operator=(const VoxelGrid& grid) = delete;

    const VoxelGridOptions& m_options;

    float m_dim_x;
    float m_dim_x_lower;
    float m_dim_x_upper;

    float m_dim_y;
    float m_dim_y_lower;
    float m_dim_y_upper;

    float m_dim_z;
    float m_dim_z_lower;
    float m_dim_z_upper;

    float m_res_x;
    float m_res_y;
    float m_res_z;

    float m_nx;
    float m_ny;
    float m_nz;

    float offset_x;
    float offset_y;
    float offset_z;

    std::vector<Voxel*> m_voxels;
};

void serialize(const VoxelGrid& grid, VoxelGridProto& proto);

void deserialize(const VoxelGridProto& proto, std::vector<Voxel>& grid);

void pointsToVoxels(VoxelGrid& grid, Eigen::MatrixXf& points, std::vector<const Voxel*>& voxels);

bool angleToVertical(const Voxel& voxel, float& angle);

} // perception
