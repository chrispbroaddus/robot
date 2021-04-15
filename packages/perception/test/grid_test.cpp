#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/perception/grid.h"

#include <chrono>
#include <random>

constexpr float kMeanTolerance = 0.01;

TEST(grid, voxelGridPerformance) {
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Point3f;

    constexpr float half_grid = 5.0;
    VoxelGridOptions options;
    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);

    options.mutable_grid_options()->set_res_x(0.2);
    options.mutable_grid_options()->set_res_y(0.2);
    options.mutable_grid_options()->set_res_z(0.2);

    VoxelGrid grid(options);

    int points = 0;

    auto start = std::chrono::steady_clock::now();
    for (float i = -half_grid; i < half_grid; i += 0.1f) {
        for (float j = -half_grid; j < half_grid; j += 0.1f) {
            for (float k = -half_grid; k < half_grid; k += 0.1f) {
                PointCloudXYZ cloud;
                cloud.xyz.resize(3, 1);
                cloud.xyz.block<3, 1>(0, 0) = Point3f({ i, j, k });
                grid.add(cloud);
                ++points;
            }
        }
    }
    auto diff = std::chrono::steady_clock::now() - start;
    auto delta = std::chrono::duration<double, std::milli>(diff).count();
    LOG(INFO) << points << " in " << delta << " ms";
    LOG(INFO) << (points / (delta / 1000)) << " points/second";
}

TEST(grid, voxelGridAddPointsCheck) {
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Point3f;

    constexpr float half_grid = 5.0;
    VoxelGridOptions options;

    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);

    options.mutable_grid_options()->set_res_x(1.0);
    options.mutable_grid_options()->set_res_y(1.0);
    options.mutable_grid_options()->set_res_z(1.0);

    VoxelGrid grid(options);

    for (float i = -half_grid; i < half_grid; i += 0.1f) {
        PointCloudXYZ cloud;
        cloud.xyz.resize(3, 1);
        cloud.xyz.block<3, 1>(0, 0) = Point3f({ i, 0, 0 });
        grid.add(cloud);
    }
    for (int i = 0; i < 10; ++i) {
        ASSERT_GT(grid(i, 5, 5)->n, 0);
    }
    ASSERT_EQ(grid(0, 0, 0)->n, 0);
}

TEST(grid, voxelGridExtentsCheck) {
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Point3f;

    constexpr float half_grid = 5.0;
    VoxelGridOptions options;

    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);
    options.mutable_grid_options()->set_res_x(1.0);
    options.mutable_grid_options()->set_res_y(1.0);
    options.mutable_grid_options()->set_res_z(1.0);
    VoxelGrid grid(options);

    std::vector<Point3f> points;
    for (float x = 20; x < 30; x += .1f) {
        for (float y = 20; y < 30; y += .1f) {
            for (float z = 20; z < 30; z += .1f) {
                points.push_back(Point3f{ x, y, z });
            }
        }
    }
    PointCloudXYZ cloud;
    cloud.xyz = Eigen::MatrixXf(3, points.size());
    int counter = 0;
    for (auto point : points) {
        cloud.xyz.col(counter++) = point;
    }
    grid.add(cloud);
    for (const auto* voxel : grid.voxels()) {
        ASSERT_EQ(voxel->n, 0);
    }
}

TEST(grid, voxelSingleHorizontalSurfletCheck) {
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Point3f;

    constexpr float half_grid = 1.5;
    VoxelGridOptions options;

    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);

    options.mutable_grid_options()->set_res_x(1.0);
    options.mutable_grid_options()->set_res_y(1.0);
    options.mutable_grid_options()->set_res_z(1.0);

    VoxelGrid grid(options);
    ASSERT_EQ(grid.voxels().size(), 3 * 3 * 3);

    constexpr float kGridDelta = 0.01;

    std::random_device device;
    std::mt19937 generator(device());
    std::normal_distribution<> distribution(0, .01);

    constexpr int kDefaultNumberPoints = 100;
    PointCloudXYZ cloud;
    cloud.xyz.resize(3, kDefaultNumberPoints);

    for (int i = 0; i < kDefaultNumberPoints; ++i) {
        float x = distribution(generator);
        float y = distribution(generator);
        float z = distribution(generator);
        x = std::max(x, (-half_grid + kGridDelta));
        x = std::min(x, (half_grid - kGridDelta));
        y = std::max(y, (-half_grid + kGridDelta));
        y = std::min(y, (half_grid - kGridDelta));
        z /= 100;
        cloud.xyz.block<3, 1>(0, i) = Point3f({ x, y, z });
    }
    CHECK(grid.add(cloud));

    auto voxels = grid.voxels();
    auto populated = std::count_if(voxels.begin(), voxels.end(), [](perception::Voxel* voxel) { return voxel->n > 0; });
    ASSERT_EQ(populated, 1);

    grid.computeSurflets();

    auto mean = grid(1, 1, 1)->mean;
    ASSERT_LT(fabs(mean[0]), kMeanTolerance);
    ASSERT_LT(fabs(mean[1]), kMeanTolerance);
    ASSERT_LT(mean[2], kMeanTolerance);

    auto variance = grid(1, 1, 1)->variance;
    ASSERT_LT(fabs(variance(0, 0)), pow(kMeanTolerance, 2));
    ASSERT_LT(fabs(variance(1, 1)), pow(kMeanTolerance, 2));
    ASSERT_LT(fabs(variance(2, 2)), pow(kMeanTolerance, 2));

    auto normal = grid(1, 1, 1)->normal;
    ASSERT_GT(fabs(normal[2]), fabs(normal[1]));
    ASSERT_GT(fabs(normal[2]), fabs(normal[0]));
    ASSERT_NEAR(fabs(normal[2]), 1.0, 0.05);
}

TEST(grid, voxelSingleVerticalSurfletCheck) {
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Point3f;

    constexpr float half_grid = 1.5;
    VoxelGridOptions options;

    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);

    options.mutable_grid_options()->set_res_x(1.0);
    options.mutable_grid_options()->set_res_y(1.0);
    options.mutable_grid_options()->set_res_z(1.0);

    VoxelGrid grid(options);
    ASSERT_EQ(grid.voxels().size(), 3 * 3 * 3);

    constexpr float kGridDelta = 0.01;

    std::random_device device;
    std::mt19937 generator(device());
    std::normal_distribution<> distribution(0, .01);

    constexpr int kDefaultNumberPoints = 100;
    PointCloudXYZ cloud;
    cloud.xyz.resize(3, kDefaultNumberPoints);

    // Generate a truncated gaussian
    for (int i = 0; i < kDefaultNumberPoints; ++i) {
        float x = distribution(generator);
        float y = distribution(generator);
        float z = distribution(generator);
        x /= 100;
        y = std::max(y, (-half_grid + kGridDelta));
        y = std::min(y, (half_grid - kGridDelta));
        z = std::max(z, (-half_grid + kGridDelta));
        z = std::min(z, (half_grid - kGridDelta));

        cloud.xyz.block<3, 1>(0, i) = Point3f({ x, y, z });
    }
    grid.add(cloud);

    auto voxels = grid.voxels();
    auto populated = std::count_if(voxels.begin(), voxels.end(), [](perception::Voxel* voxel) { return voxel->n > 0; });
    ASSERT_EQ(populated, 1);
    auto mean = grid(1, 1, 1)->mean;
    ASSERT_LT(fabs(mean[0]), kMeanTolerance);
    ASSERT_LT(fabs(mean[1]), kMeanTolerance);
    ASSERT_LT(mean[2], kMeanTolerance);

    auto variance = grid(1, 1, 1)->variance;
    ASSERT_LT(fabs(variance(0, 0)), pow(kMeanTolerance, 2));
    ASSERT_LT(fabs(variance(1, 1)), pow(kMeanTolerance, 2));
    ASSERT_LT(fabs(variance(2, 2)), pow(kMeanTolerance, 2));

    grid.computeSurflets();

    auto normal = grid(1, 1, 1)->normal;
    ASSERT_GT(fabs(normal[0]), fabs(normal[1]));
    ASSERT_GT(fabs(normal[0]), fabs(normal[2]));
    ASSERT_NEAR(fabs(normal[0]), 1.0, 0.05);
}

TEST(grid, voxelSurfletCheck) {
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Point3f;

    constexpr float half_grid = 1.5;
    VoxelGridOptions options;

    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);

    options.mutable_grid_options()->set_res_x(1.0);
    options.mutable_grid_options()->set_res_y(1.0);
    options.mutable_grid_options()->set_res_z(1.0);

    VoxelGrid grid(options);
    ASSERT_EQ(grid.voxels().size(), 3 * 3 * 3);

    constexpr float kGridDelta = 0.01;

    std::random_device device;
    std::mt19937 generator(device());
    std::normal_distribution<> distribution(0, 1);

    constexpr int kDefaultNumberPoints = 100000;
    PointCloudXYZ cloud;
    cloud.xyz.resize(3, kDefaultNumberPoints);

    for (int i = 0; i < kDefaultNumberPoints; ++i) {
        float x = distribution(generator);
        float y = distribution(generator);
        float z = distribution(generator);
        x = std::max(x, (-half_grid + kGridDelta));
        x = std::min(x, (half_grid - kGridDelta));
        y = std::max(y, (-half_grid + kGridDelta));
        y = std::min(y, (half_grid - kGridDelta));
        z = std::max(z, (-half_grid + kGridDelta));
        z = std::min(z, (half_grid - kGridDelta));

        cloud.xyz.block<3, 1>(0, i) = Point3f({ x, y, z });
    }
    grid.add(cloud);

    auto voxels = grid.voxels();
    auto populated = std::count_if(voxels.begin(), voxels.end(), [](perception::Voxel* voxel) { return voxel->n > 0; });
    ASSERT_EQ(populated, 3 * 3 * 3);

    auto start = std::chrono::steady_clock::now();
    grid.computeSurflets();
    auto diff = std::chrono::steady_clock::now() - start;
    auto delta = std::chrono::duration<double, std::milli>(diff).count();
    LOG(INFO) << (3 * 3 * 3) << " surflets in " << delta << " ms";
}

TEST(grid, serializationCheck) {
    using perception::VoxelGridProto;
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Point3f;
    using perception::Voxel;

    constexpr float half_grid = 5.0;
    VoxelGridOptions options;

    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);

    options.mutable_grid_options()->set_res_x(1.0);
    options.mutable_grid_options()->set_res_y(1.0);
    options.mutable_grid_options()->set_res_z(1.0);

    VoxelGrid grid(options);

    for (float i = -half_grid; i < half_grid; i++) {
        for (float j = -half_grid; j < half_grid; j++) {
            for (float k = -half_grid; k < half_grid; k++) {
                PointCloudXYZ cloud;
                cloud.xyz.resize(3, 1);
                cloud.xyz.block<3, 1>(0, 0) = Point3f({ i, j, k });
                ASSERT_TRUE(grid.add(cloud));
            }
        }
    }
    VoxelGridProto serialized;
    perception::serialize(grid, serialized);
    std::vector<Voxel> voxels;
    perception::deserialize(serialized, voxels);
    for (auto voxel : voxels) {
        ASSERT_EQ(voxel.n, 1);
    }
}

TEST(grid, indexCheck) {
    using perception::pointsToVoxels;
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::PointCloudXYZ;
    using perception::Voxel;
    using perception::Point3f;
    constexpr float half_grid = 5.0;

    VoxelGridOptions options;

    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);

    options.mutable_grid_options()->set_res_x(1.0);
    options.mutable_grid_options()->set_res_y(1.0);
    options.mutable_grid_options()->set_res_z(1.0);

    VoxelGrid grid(options);
    Eigen::MatrixXf points;
    points.resize(3, 0);
    std::vector<const Voxel*> voxels;
    pointsToVoxels(grid, points, voxels);
    ASSERT_EQ(voxels.size(), 0);

    std::vector<Eigen::Vector3f> gridded_points;

    for (float i = -half_grid; i < half_grid; i += 0.1f) {
        for (float j = -half_grid; j < half_grid; j += 0.1f) {
            for (float k = -half_grid; k < half_grid; k += 0.1f) {
                gridded_points.emplace_back(Point3f({ i, j, k }));
            }
        }
    }

    points.resize(3, gridded_points.size());
    int counter = 0;
    for (const auto& point : gridded_points) {
        points.block<3, 1>(0, counter++) = Point3f({ point[0], point[1], point[2] });
    }
    pointsToVoxels(grid, points, voxels);
    ASSERT_EQ(voxels.size(), gridded_points.size());
    for (const auto& voxel : voxels) {
        ASSERT_NE(voxel, nullptr);
    }
}

TEST(grid, voxelVerticalAngleCheck) {
    using perception::Voxel;
    using perception::angleToVertical;
    Voxel voxel;
    float result;

    voxel.normal = Eigen::Vector3f({ 0, 0, 1 });
    ASSERT_TRUE(angleToVertical(voxel, result));
    ASSERT_EQ(result, 0.0);

    voxel.normal = Eigen::Vector3f({ 0, 1, 1 });
    ASSERT_FALSE(angleToVertical(voxel, result));

    voxel.normal = Eigen::Vector3f({ 0, 0, -1 });
    ASSERT_TRUE(angleToVertical(voxel, result));
    ASSERT_FLOAT_EQ(result, M_PI);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
