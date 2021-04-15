#include "packages/perception/utils.h"

#include "packages/perception/types.h"

#include <fstream>

namespace perception {

static constexpr float kDefaultMaxRange = 100.0;

void transform(PointCloudXYZ& cloud, const Sophus::SE3d& transform) {
    Eigen::Matrix3f rotation = transform.so3().matrix().cast<float>();
    Eigen::MatrixXf result = cloud.xyz.transpose() * rotation;
    cloud.xyz = result.transpose();
}

bool imageToPointCloud(const hal::Image& image, core::PointCloud3d& cloud) {
    const int rows = image.rows();
    const int cols = image.cols();
    const int dimension = image.stride();
    const int num_points = rows * cols;
    CHECK(rows > 0);
    CHECK(cols > 0);
    CHECK(3 == dimension);
    std::vector<float> image_data;
    const float* data = reinterpret_cast<const float*>(image.data().data());
    std::copy(data, data + (dimension * num_points), std::back_inserter(image_data));
    CHECK(image_data.size() == static_cast<size_t>(rows * cols * dimension));
    cloud.clear_points();

    for (int i = 0; i < num_points; ++i) {
        auto pt_x = image_data[i * dimension];
        auto pt_y = image_data[i * dimension + 1];
        auto pt_z = image_data[i * dimension + 2];
        auto pt = cloud.add_points();
        pt->set_x(pt_x);
        pt->set_y(pt_y);
        pt->set_z(pt_z);
    }

    return (cloud.points_size() == num_points);
}

bool imageToPointCloud(const hal::Image& image, PointCloudXYZ& cloud) {
    cloud.xyz = Eigen::Map<const Eigen::MatrixXf, Eigen::RowMajor>(
        reinterpret_cast<const float*>(image.data().data()), 3, image.rows() * image.cols());
    return (cloud.xyz.cols() > 0);
}

UnprojectionLookup::UnprojectionLookup(std::shared_ptr<calibu::CameraInterface<double> > interface)
    : x_dim(0)
    , y_dim(0) {
    core::Point3d centre;
    centre.set_x(0);
    centre.set_y(0);
    centre.set_z(-1);
    x_dim = interface->Width();
    y_dim = interface->Height();
    CHECK(x_dim > 0);
    CHECK(y_dim > 0);
    Eigen::initParallel();

    m_rayLookup = Eigen::ArrayXXf(6, interface->Width() * interface->Height());

    const int cols_ = interface->Width();
    const int rows_ = interface->Height();

    for (int i = 0; i < cols_; ++i) {
        for (int j = 0; j < rows_; ++j) {
            auto _ray = interface->Unproject(Eigen::Vector2d({ i, j }));
            m_rayLookup.block<3, 1>(0, i * rows_ + j) = Eigen::Matrix<float, 1, 3>({ 0, 0, 0 });
            m_rayLookup.block<3, 1>(3, i * rows_ + j) = _ray.cast<float>();
        }
    }
}

void UnprojectionLookup::unproject(const hal::Image& image, PointCloudXYZ& cloud) const {
    const int cols = image.cols();
    const int rows = image.rows();
    const int num_points = rows * cols;
    CHECK(cloud.xyz.rows() == 3);
    CHECK(cloud.xyz.cols() == num_points);
    CHECK(static_cast<int>(image.cols()) == x_dim);
    CHECK(static_cast<int>(image.rows()) == y_dim);

    const float* range_data = reinterpret_cast<const float*>(image.data().data());
    auto ranges = Eigen::Map<const Eigen::VectorXf, 0, Eigen::InnerStride<1> >(range_data, num_points);
#pragma omp parallel for
    for (int i = 0; i < num_points; ++i) {
        if (ranges(i) > kDefaultMaxRange || ranges(i) < 0) {
            continue;
        }
        cloud.xyz.block<3, 1>(0, i) = m_rayLookup.block<3, 1>(3, i) * ranges(i);
    }
    CHECK(num_points == cloud.xyz.cols());
}

void saveCloud(const PointCloudXYZ& cloud) {
    static int sDumpCounter = 0;
    CHECK(cloud.xyz.rows() == 3);
    std::stringstream output;
    output << "stereo_live_" << sDumpCounter++ << ".txt";
    Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
    std::ofstream file(output.str());
    CHECK(file.good());
    file << cloud.xyz.format(CleanFmt);
    file.close();
}

void checkCloud(const PointCloudXYZ& cloud, const int num_expected_points) {
    CHECK(cloud.xyz.rows() == 3);
    CHECK(cloud.xyz.cols() == num_expected_points);
    int invalid = 0;
    for (int i = 0; i < num_expected_points; ++i) {
        auto point = cloud.xyz.col(i);
        CHECK(point[1] >= 0) << point[1];
        if (std::isinf(point[0]) || std::isinf(point[1]) || std::isinf(point[2])) {
            ++invalid;
        }
        if (std::isnan(point[0]) || std::isnan(point[0]) || std::isnan(point[0])) {
            LOG(FATAL) << "Invalid data at index: " << i;
        }
    }
    auto ratio = static_cast<float>(invalid) / num_expected_points;
    CHECK(ratio < kMaxAllowableInvalidCloudPoints) << ratio * 100.0f << "% invalid points";
}

calibration::SystemCalibration generateDummySystemCalibration() {
    calibration::SystemCalibration calibration;

    constexpr int kDefaultNumCameras = 10;

    // Add intrinsics
    for (int i = 0; i < kDefaultNumCameras; ++i) {
        auto intrinsic = calibration.add_cameraintrinsiccalibration();

        hal::Device device;
        device.set_name("camera" + std::to_string(i));
        *intrinsic->mutable_cameraundercalibration() = device;

        constexpr int kDefaultWidth = 2000;
        constexpr int kDefaultHeight = 1000;
        constexpr int kDefaultFocalLengthX = 100;
        constexpr int kDefaultFocalLengthY = 50;

        if (i % 2 == 0) {
            intrinsic->set_opticalcenterx(kDefaultWidth / 4);
            intrinsic->set_opticalcentery(kDefaultHeight / 4);
            intrinsic->set_scaledfocallengthx(kDefaultFocalLengthX / 4);
            intrinsic->set_scaledfocallengthy(kDefaultFocalLengthY / 4);
            intrinsic->set_resolutionx(kDefaultWidth / 2);
            intrinsic->set_resolutiony(kDefaultHeight / 2);

            calibration::PinholeCameraDistortionModel pinhole;
            *intrinsic->mutable_pinhole() = pinhole;
        } else {
            intrinsic->set_opticalcenterx(kDefaultWidth / 2);
            intrinsic->set_opticalcentery(kDefaultHeight / 2);
            intrinsic->set_scaledfocallengthx(kDefaultFocalLengthX / 2);
            intrinsic->set_scaledfocallengthy(kDefaultFocalLengthY / 2);
            intrinsic->set_resolutionx(kDefaultWidth);
            intrinsic->set_resolutiony(kDefaultHeight);

            calibration::KannalaBrandtDistortionModel kb;
            *intrinsic->mutable_kannalabrandt() = kb;
            for (int i = 0; i < 4; ++i) {
                intrinsic->mutable_kannalabrandt()->add_radialdistortioncoefficientk(i);
            }
        }
    }

    hal::Device robot;
    robot.set_name("robot");
    calibration::CoordinateFrame robot_frame;
    *robot_frame.mutable_device() = robot;

    for (int i = 0; i < kDefaultNumCameras; ++i) {
        auto extrinsic = calibration.add_devicetodevicecoordinatetransformation();

        hal::Device camera_device;
        camera_device.set_name("camera" + std::to_string(i));
        calibration::CoordinateFrame camera_frame;
        *camera_frame.mutable_device() = camera_device;

        *extrinsic->mutable_sourcecoordinateframe() = robot_frame;
        *extrinsic->mutable_targetcoordinateframe() = camera_frame;

        extrinsic->set_rodriguesrotationx(i);
        extrinsic->set_rodriguesrotationy(i);
        extrinsic->set_rodriguesrotationz(i);

        extrinsic->set_translationx(i + 1);
        extrinsic->set_translationy(i + 1);
        extrinsic->set_translationz(i + 1);
    }
    return calibration;
}

calibration::SystemCalibration generateBasic4CameraSystemCalibration() {
    calibration::SystemCalibration calibration;

    constexpr int kDefaultNumCameras = 4;

    // Add intrinsics
    for (int i = 0; i < kDefaultNumCameras; ++i) {
        auto intrinsic = calibration.add_cameraintrinsiccalibration();

        hal::Device device;
        device.set_name("camera" + std::to_string(i));
        *intrinsic->mutable_cameraundercalibration() = device;

        constexpr int kDefaultWidth = 2000;
        constexpr int kDefaultHeight = 1000;
        constexpr int kDefaultFocalLengthX = 100;
        constexpr int kDefaultFocalLengthY = 50;

        intrinsic->set_opticalcenterx(kDefaultWidth / 4);
        intrinsic->set_opticalcentery(kDefaultHeight / 4);
        intrinsic->set_scaledfocallengthx(kDefaultFocalLengthX / 4);
        intrinsic->set_scaledfocallengthy(kDefaultFocalLengthY / 4);
        intrinsic->set_resolutionx(kDefaultWidth / 2);
        intrinsic->set_resolutiony(kDefaultHeight / 2);

        calibration::PinholeCameraDistortionModel pinhole;
        *intrinsic->mutable_pinhole() = pinhole;
    }

    hal::Device robot;
    robot.set_name("robot");
    calibration::CoordinateFrame robot_frame;
    *robot_frame.mutable_device() = robot;

    float orientation = 0.0;

    for (int i = 0; i < kDefaultNumCameras; ++i) {
        auto extrinsic = calibration.add_devicetodevicecoordinatetransformation();

        hal::Device camera_device;
        camera_device.set_name("camera" + std::to_string(i));
        calibration::CoordinateFrame camera_frame;
        *camera_frame.mutable_device() = camera_device;

        *extrinsic->mutable_sourcecoordinateframe() = robot_frame;
        *extrinsic->mutable_targetcoordinateframe() = camera_frame;

        Eigen::Vector3d rotation{ 0, 0, orientation };
        orientation += static_cast<float>(M_PI) / 2;
        extrinsic->set_rodriguesrotationx(rotation[0]);
        extrinsic->set_rodriguesrotationy(rotation[1]);
        extrinsic->set_rodriguesrotationz(rotation[2]);

        extrinsic->set_translationx(0);
        extrinsic->set_translationy(0);
        extrinsic->set_translationz(-1);
    }
    return calibration;
}

void cvToZippy(const Eigen::Vector3d& from, Eigen::Vector3d& to) {
    to[0] = from[2];
    to[1] = from[0];
    to[2] = from[1];
}

} // planner
