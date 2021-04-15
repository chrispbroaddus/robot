#include "gtest/gtest.h"

#include "packages/perception/terrain.h"

TEST(UtilsTest, GridIntersection) {
    using perception::FlatTerrain;
    std::unique_ptr<FlatTerrain> terrain(new FlatTerrain());
    core::Point3d origin;
    origin.set_x(0.0);
    origin.set_y(0.0);
    origin.set_z(2.0);
    core::Point3d intersection;

    {
        core::Ray3d ray;
        ray.mutable_direction()->set_x(1.0);
        ray.mutable_direction()->set_y(0.0);
        ray.mutable_direction()->set_z(0.0);
        *ray.mutable_origin() = origin;
        ASSERT_FALSE(terrain->intersect(ray, intersection));
    }

    {
        core::Ray3d ray;
        ray.mutable_direction()->set_x(0.0);
        ray.mutable_direction()->set_y(0.0);
        ray.mutable_direction()->set_z(1.0);
        *ray.mutable_origin() = origin;
        ASSERT_FALSE(terrain->intersect(ray, intersection));
    }

    {
        core::Ray3d ray;
        ray.mutable_direction()->set_x(0.0);
        ray.mutable_direction()->set_y(0.0);
        ray.mutable_direction()->set_z(-1.0);
        *ray.mutable_origin() = origin;
        ASSERT_TRUE(terrain->intersect(ray, intersection));
        ASSERT_TRUE(intersection.x() == 0);
        ASSERT_TRUE(intersection.y() == 0);
    }
}

TEST(VoxelTest, Obstruction) {
    using perception::pointsToVoxels;
    using perception::VoxelTerrain;
    using perception::VoxelTerrainOptions;
    VoxelTerrainOptions options;
    options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_x(20.0);
    options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_y(20.0);
    options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_z(20.0);
    options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_x(1.0);
    options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_y(1.0);
    options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_z(1.0);

    std::unique_ptr<VoxelTerrain> terrain(new VoxelTerrain(options));
    auto& grid = terrain->grid();

    // Artificially add horizontal voxel normal
    for (auto voxel : grid.voxels()) {
        voxel->normal = Eigen::Vector3f({ 0, 0, 1 });
    }
    // Check: angleToVertical
    for (const auto* voxel : grid.voxels()) {
        float angle = 0;
        CHECK(angleToVertical(*voxel, angle));
        ASSERT_FLOAT_EQ(angle, 0);
    }
    std::vector<core::Point3d> path;
    for (int i = -10; i < 10; ++i) {
        core::Point3d point;
        point.set_x(i);
        point.set_y(i);
        point.set_z(0);
        path.emplace_back(point);
    }
    ASSERT_TRUE(terrain->unobstructed(path));

    // Artificially add vertical voxel normal
    for (auto voxel : grid.voxels()) {
        voxel->normal = Eigen::Vector3f({ 0, 1, 0 });
    }
    ASSERT_FALSE(terrain->unobstructed(path));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
