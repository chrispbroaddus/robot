#include "packages/perception/grid.h"

#include <cmath>

#include "Eigen/Eigenvalues"
#include "glog/logging.h"

namespace perception {

static constexpr int kDefaultInvalidRange = 10000;
static constexpr int kMinimumPointsPerVoxel = 10;
static constexpr float kNormalTolerance = 0.01;

inline int xyzToIdx(float x, float y, float z, float res_x, float res_y, float res_z, int num_x, int num_y,
    __attribute__((unused)) int num_z, float offset_x, float offset_y, float offset_z) {
    auto i = static_cast<int>(std::floor((x - offset_x) / res_x));
    auto j = static_cast<int>(std::floor(floor((y - offset_y) / res_y)));
    auto k = static_cast<int>(std::floor(floor((z - offset_z) / res_z)));
    return i * (num_x * num_y) + j * (num_y) + k;
}

void initialiseVoxel(Voxel* voxel) {
    CHECK_NOTNULL(voxel);
    voxel->n = 0;
    voxel->valid = false;
    // voxel-centre remains unchanged
    voxel->mean.setZero();
    voxel->variance.setZero();
    voxel->normal.setZero();
    // Note: voxel->observations remains partially populated; we rely on
    //       voxel->n being set appropriately;
}

VoxelGrid::VoxelGrid(const VoxelGridOptions& options)
    : m_options(options)
    , m_dim_x(options.grid_options().dim_x())
    , m_dim_y(options.grid_options().dim_y())
    , m_dim_z(options.grid_options().dim_z())
    , m_res_x(options.grid_options().res_x())
    , m_res_y(options.grid_options().res_y())
    , m_res_z(options.grid_options().res_z()) {
    CHECK(m_dim_x > 0);
    m_dim_x_lower = -1 * m_dim_x / 2;
    m_dim_x_upper = m_dim_x / 2;

    CHECK(m_dim_y > 0);
    m_dim_y_lower = -1 * m_dim_y / 2;
    m_dim_y_upper = m_dim_y / 2;

    CHECK(m_dim_z > 0);
    m_dim_z_lower = -1 * m_dim_z / 2;
    m_dim_z_upper = m_dim_z / 2;

    CHECK(m_res_x > 0);
    CHECK(m_res_y > 0);
    CHECK(m_res_z > 0);
    m_nx = ceil(m_dim_x / m_res_x);
    m_ny = ceil(m_dim_y / m_res_y);
    m_nz = ceil(m_dim_z / m_res_z);
    CHECK(m_nx > 0);
    CHECK(m_ny > 0);
    CHECK(m_nz > 0);
    offset_x = -1 * m_dim_x / 2;
    offset_y = -1 * m_dim_y / 2;
    offset_z = -1 * m_dim_z / 2;
    for (int xi = 0; xi < m_nx; ++xi) {
        for (int yi = 0; yi < m_ny; ++yi) {
            for (int zi = 0; zi < m_nz; ++zi) {
                float x = offset_x + xi * m_res_x + m_res_x / 2;
                float y = offset_y + yi * m_res_y + m_res_y / 2;
                float z = offset_z + zi * m_res_z + m_res_z / 2;
                Voxel* voxel = new Voxel;
                CHECK_NOTNULL(voxel);
                voxel->centre = Eigen::Vector3f({ x, y, z });
                m_voxels.push_back(voxel);
            }
        }
    }
    clear();
}

VoxelGrid::~VoxelGrid() {
    for (auto& voxel : m_voxels) {
        CHECK_NOTNULL(voxel);
        delete voxel;
        voxel = nullptr;
    }
}

void VoxelGrid::clear() {
    for (auto& voxel : m_voxels) {
        initialiseVoxel(voxel);
    }
}

bool VoxelGrid::add(const PointCloudXYZ& cloud) {
    CHECK(cloud.xyz.rows() == 3);
    if (cloud.xyz.cols() == 0) {
        return false;
    }
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;

    for (int col = 0; col < cloud.xyz.cols(); ++col) {
        auto point = cloud.xyz.col(col);
        x = point[0];
        y = point[1];
        z = point[2];
        if (x < m_dim_x_lower || x > m_dim_x_upper) {
            continue;
        }
        if (y < m_dim_y_lower || y > m_dim_y_upper) {
            continue;
        }
        if (z < m_dim_z_lower || z > m_dim_z_upper) {
            continue;
        }
        auto idx = xyzToIdx(x, y, z, m_res_x, m_res_y, m_res_z, m_nx, m_ny, m_nz, offset_x, offset_y, offset_z);
        CHECK(idx >= 0) << x << ", " << y << ", " << z;
        CHECK(idx < static_cast<int>(m_voxels.size())) << x << ", " << y << ", " << z;
        auto voxel = m_voxels[idx];
        CHECK_NOTNULL(voxel);
        if (voxel->n == (kMaxObservationsPerVoxel - 1)) {
            continue;
        }
        CHECK(voxel->n < kMaxObservationsPerVoxel);
        voxel->observations.row(voxel->n) = Eigen::Vector3f{ x, y, z };
        voxel->n++;
    }
    return true;
}

const Voxel* VoxelGrid::operator()(int i, int j, int k) const {
    CHECK(i >= 0);
    CHECK(j >= 0);
    CHECK(k >= 0);
    auto idx = i * (m_nx * m_ny) + j * (m_ny) + k;
    CHECK(idx < m_voxels.size());
    return m_voxels[idx];
}

const Voxel* VoxelGrid::pointToVoxel(const Eigen::Vector3f& point) const {
    auto idx = xyzToIdx(point[0], point[1], point[2], m_res_x, m_res_y, m_res_z, m_nx, m_ny, m_nz, offset_x, offset_y, offset_z);
    if (idx < 0 || idx > static_cast<int>(m_voxels.size())) {
        LOG(ERROR) << "Failed to index: " << point << " (idx: " << idx << "), (#voxels: " << m_voxels.size() << ")";
        return nullptr;
    }
    return m_voxels[idx];
}

void VoxelGrid::computeSurflets() {
#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(m_voxels.size()); ++i) {
        auto* voxel = m_voxels[i];
        CHECK_NOTNULL(voxel);
        if (voxel->n >= 3) {
            auto observations = voxel->observations.topLeftCorner(voxel->n, 3);
            voxel->mean = observations.colwise().mean();
            if (voxel->n >= kMinObservationsPerSurflet) {
                voxel->valid = true;
                Eigen::MatrixXf centered = observations.rowwise() - observations.colwise().mean();
                Eigen::MatrixXf variance = (centered.adjoint() * centered) / double(voxel->n - 1);
                auto components = Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f>(variance);
                Eigen::MatrixXf::Index index, col_;
                components.eigenvalues().minCoeff(&index, &col_);
                voxel->normal = components.eigenvectors().block<3, 1>(0, index);
            }
        }
    }
}

void pointsToVoxels(VoxelGrid& grid, Eigen::MatrixXf& points, std::vector<const Voxel*>& voxels) {
    CHECK(3 == points.rows());
    voxels.clear();
    auto num_points = points.cols();
    voxels.reserve(num_points);
    for (int i = 0; i < num_points; ++i) {
        voxels.push_back(grid.pointToVoxel(points.col(i)));
    }
}

bool angleToVertical(const Voxel& voxel, float& angle) {
    if (voxel.normal.norm() - 1 > kNormalTolerance) {
        return false;
    }
    static const auto kUnitZ = Eigen::Vector3f({ 0, 0, 1 });
    angle = std::acos(voxel.normal.dot(kUnitZ));
    return true;
}

void serialize(const VoxelGrid& grid, VoxelGridProto& proto) {
    *proto.mutable_options() = grid.options();

    const auto voxels = grid.voxels();
    std::vector<Voxel*> filledVoxels;
    std::copy_if(voxels.begin(), voxels.end(), std::back_inserter(filledVoxels), [](const Voxel* voxel) -> bool { return voxel->n > 0; });
    std::vector<Voxel> voxelCopies;
    voxelCopies.reserve(filledVoxels.size());

    for (const auto* voxel : filledVoxels) {
        voxelCopies.push_back(*voxel);
    }
    proto.set_num_voxels(voxelCopies.size());
    proto.set_voxels(voxelCopies.data(), sizeof(Voxel) * voxelCopies.size());
}

void deserialize(const VoxelGridProto& proto, std::vector<Voxel>& grid) {
    if (proto.num_voxels() <= 0) {
        return;
    }
    grid.clear();
    grid.insert(grid.begin(), reinterpret_cast<const Voxel*>(proto.voxels().data()),
        reinterpret_cast<const Voxel*>(proto.voxels().data()) + proto.num_voxels());
}

} // perception
