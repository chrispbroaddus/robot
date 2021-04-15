#include "packages/perception/terrain.h"

#include <chrono>

#include "packages/core/include/chrono.h"

#include "wykobi/wykobi.hpp"

namespace perception {

static constexpr float kSurfletOrientationTolerance = 0.1;
static const wykobi::point3d<double> kOrigin = wykobi::make_point(0.0, 0.0, 0.0);
static const wykobi::vector3d<double> kUnitZ = wykobi::make_vector(0.0, 0.0, 1.0);
static const wykobi::plane<double, 3> kXYPlane = wykobi::make_plane(kOrigin, kUnitZ);

bool FlatTerrain::intersect(const core::Ray3d& ray, core::Point3d& intersection_point) {
    wykobi::point3d<double> _start;
    _start.x = ray.origin().y();
    _start.y = ray.origin().x();
    _start.z = -1 * ray.origin().z();

    wykobi::vector3d<double> _direction;
    _direction.x = ray.direction().y();
    _direction.y = ray.direction().x();
    _direction.z = -1 * ray.direction().z();

    auto _ray = wykobi::make_ray(_start, _direction);
    if (!wykobi::intersect(_ray, kXYPlane)) {
        return false;
    }
    auto _intersection_point = wykobi::intersection_point(_ray, kXYPlane);
    intersection_point.set_x(_intersection_point.y);
    intersection_point.set_y(_intersection_point.x);
    CHECK(fabs(_intersection_point.z) < 1e-15);
    intersection_point.set_z(0.0);
    return true;
}

VoxelTerrain::VoxelTerrain(const VoxelTerrainOptions& options)
    : m_options(options) {
    m_grid.reset(new VoxelGrid(m_options.voxel_grid_options()));
}

bool VoxelTerrain::addPoints(const PointCloudXYZ& points) {
    using core::chrono::gps::wallClockInNanoseconds;
    CHECK(points.xyz.rows() == 3);
    CHECK(points.xyz.cols() > 0);
    auto begin = wallClockInNanoseconds();
    auto retval = m_grid->add(points);
    auto add = wallClockInNanoseconds();
    m_grid->computeSurflets();
    auto surflets = wallClockInNanoseconds();
    LOG(INFO) << "Grid (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(add - begin).count();
    LOG(INFO) << "Surflets (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(surflets - add).count();
    return retval;
}

bool VoxelTerrain::unobstructed(const std::vector<core::Point3d>& path) {
    CHECK(!path.empty());
    Eigen::MatrixXf points(3, path.size());

    int counter = 0;
    for (const auto& entry : path) {
        points.block<3, 1>(0, counter++)
            = Eigen::Vector3f({ static_cast<float>(entry.x()), static_cast<float>(entry.y()), static_cast<float>(entry.z()) });
    }
    std::vector<const Voxel*> voxels;
    pointsToVoxels(*m_grid, points, voxels);
    CHECK(!voxels.empty());
    float angle_to_vertical = 0;

    for (const auto* voxel : voxels) {
        CHECK_NOTNULL(voxel);
        CHECK(angleToVertical(*voxel, angle_to_vertical));
        if (angle_to_vertical > kSurfletOrientationTolerance) {
            return false;
        }
    }
    return true;
}

} // perception
