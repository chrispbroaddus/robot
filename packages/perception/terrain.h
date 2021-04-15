#pragma once

#include "packages/core/proto/geometry.pb.h"
#include "packages/perception/grid.h"
#include "packages/perception/proto/terrain_options.pb.h"
#include "packages/perception/types.h"
#include "packages/perception/utils.h"

namespace perception {

/**
 * @brief The terrain representation interface. All classes implementing this
 * interface should accept 3d points in the robot-frame, and provide
 * specialized intersection tests for arbitrary rays.
 */
class TerrainRepresentation {
public:
    /**
     * @brief         Populate the terrain representation given the 3d points in
     *                robot frame
     *
     * @param points  3d Points in robot frame
     *
     * @return        All points successfully added to the grid
     */
    virtual bool addPoints(const PointCloudXYZ& points) = 0;

    /**
     * @brief       Estimate the intersection point of the given ray with the
     *              underlying terrain
     *
     * @param[in] ray       Input ray in robot frame
     * @param[out] point    Estimated intersection point
     *
     * @return      The ray intersects with the underlying terrain
     */
    virtual bool intersect(const core::Ray3d& ray, core::Point3d& point) = 0;

    /**
     * @brief       Estimate whether the given points, in robot-frame, are
     *              unobstructed with respect to the estimated terrain
     *
     * @param path  Input sequence of points in robot-frame
     *
     * @return      All points are unobstructed
     */
    virtual bool unobstructed(const std::vector<core::Point3d>& path) = 0;

    virtual ~TerrainRepresentation() {}
};

/**
 * @brief A simple flat-terrain model. Primarily used for simple ray-queries
 */
class FlatTerrain : public TerrainRepresentation {
public:
    bool addPoints(__attribute__((unused)) const PointCloudXYZ& points) override { return true; }

    bool intersect(const core::Ray3d& ray, core::Point3d& point) override;

    bool unobstructed(__attribute__((unused)) const std::vector<core::Point3d>& point) { return true; }
};

/**
 * @brief A dense 3d voxel approximation of the terrain. Voxel surfaces
 * (surflets) are estimated by fitting a per-voxel plane.
 */
class VoxelTerrain : public TerrainRepresentation {
public:
    VoxelTerrain(const VoxelTerrainOptions& options);

    bool addPoints(const PointCloudXYZ& points) override;

    bool intersect(__attribute__((unused)) const core::Ray3d& ray, __attribute__((unused)) core::Point3d& point) override { return false; };

    VoxelGrid& grid() {
        CHECK_NOTNULL(m_grid.get());
        return *m_grid;
    }

    bool unobstructed(const std::vector<core::Point3d>& point);

private:
    std::unique_ptr<VoxelGrid> m_grid;
    const VoxelTerrainOptions& m_options;
};

} // perception
