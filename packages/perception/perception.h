#pragma once

#include <map>

#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/perception/terrain.h"
#include "packages/perception/utils.h"
#include "packages/teleop/proto/backend_message.pb.h"

#include "calibu/Calibu.h"
#include "calibu/cam/camera_models_crtp.h"

namespace perception {

constexpr char kDefaultCamera[] = "camera0";

class Perception {
public:
    typedef std::shared_ptr<calibu::CameraInterface<double> > CameraInterface;

    Perception(const PerceptionOptions& options, std::unique_ptr<TerrainRepresentation> terrain)
        : m_options(options)
        , m_terrain(std::move(terrain)) {}

    virtual ~Perception() {}

    /**
     * @brief Add in a camera sample, optionally with the name of the projection
     * device to use.
     *
     * @param[in] sample        Camera image sample, encoded as PB_POINTCLOUD, or PB_RANGE
     * @param[in] deviceToUse   Optional device to use. This can override the device
     *                          specified in the imagery
     *
     * @return Points successfully added to the underlying terrain
     */
    virtual bool update(const hal::CameraSample& sample, std::string* deviceToUse = nullptr);

    /**
     * @brief Given a PointAndGoCommand from an operator, convert the normalized
     * pixel values into a 3D ray in the vehicle co-ordinate system
     *
     * @param[in] camera        The camera that provided the image
     * @param[in] command       The PointAndGoCommand from the browser-side
     * @param[out] core::Ray3d  A ray originating from the camera-centre in vehicle
     *                          coordinates
     *
     * @return The ray was successfully unprojected
     */
    virtual bool imageToRay(const std::string& camera, const teleop::PointAndGoCommand& command, core::Ray3d&);

    /**
     * @brief Given a PointAndGoCommand from an operator, convert the normalized
     * pixel values into a 3D ray in the vehicle co-ordinate system and
     * intersect this ray with the underlying terrain representation to give
     * a point specified in the vehicle frame
     *
     * @param[in] camera        The camera that provided the image
     * @param[in] command       The PointAndGoCommand from the browser-side
     * @param[out] core::Point3d    The 3d point in vehicle-frame corresponding
     *                              to the intersection of this point with the
     *                              underlying terrain
     *
     * @return The ray was successfully unprojected and intersected
     */
    virtual bool imageToPoint(const std::string& camera, const teleop::PointAndGoCommand& command, core::Point3d&);

    /**
     * @brief   Return the underlying camera pair (intrinsics/Calibu camera)
     *          specified by name
     *
     * @param[in] name      Name of the camera (FrontLeftStereo, RearFisheye,...)
     * @param[out] cameras  Pair of intrinsics/Calibu camera
     *
     * @return The specified camera exists
     */
    bool camera(const std::string& name, std::pair<calibration::CameraIntrinsicCalibration, CameraInterface>& cameras);

    /**
     * @brief Create an intrinsics/Calibu camera pair. Optionally discard the
     * distortion parameters and create a linear camera (for testing)
     *
     * @param[in] camera        Camera intrinsics
     * @param[in] force_linear  Force a linear camera model
     *
     * @return The intrinsics/Calibu pair was succesfully created
     */
    bool addCamera(const calibration::CameraIntrinsicCalibration& camera, bool force_linear = false);

    /**
     * @brief Retrieve the UnprojectionLookup structure corresponding to the
     * name. This is created and cached before operation to speed up
     * disparity->point cloud creation.
     *
     * @param[in] name          Name of the projector (corresponds to the camera name)
     * @param[out] projection   The UnprojectionLookup structure
     *
     * @return
     */
    bool projector(const std::string& name, UnprojectionLookup*& projection) const;

    /**
     * @brief Retrieve extrinsics for a given camera
     *
     * @param[in] name          Name of the camera
     * @param[out] extrinsics   SE3 extrinsics of the camera
     *
     * @return
     */
    bool extrinsics(const std::string& name, Sophus::SE3d& extrinsics) const;

    /**
     * @brief Add extrinsics to a specified camera.
     *
     * @param[in] name          Name of the camera
     * @param[in] extrinsics    SE3 extrinsics of robot->camera
     *
     * @return Camera exists, and extrinsics seem valid
     */
    bool addExtrinsics(const std::string& name, const Sophus::SE3d& extrinsics);

    /**
     * @brief Given a system calibration file, attempt to populate all cameras
     * by generating the intrinsics/Calibu pair, and associating the extrinsics
     *
     * @param[in] calibration   SystemCalibration proto
     *
     * @return  More than 0 cameras were added, and each camera had extrinsics
     *          associated
     */
    bool addDevices(const calibration::SystemCalibration calibration);

private:
    bool updateCloudImage(const hal::CameraSample& sample, std::string* deviceToUse);
    bool updateRangeImage(const hal::CameraSample& sample, std::string* deviceToUse);

    const PerceptionOptions& m_options;
    std::map<std::string, std::pair<calibration::CameraIntrinsicCalibration, CameraInterface> > m_cameras;
    std::map<std::string, Sophus::SE3d> m_extrinsics;

    std::unique_ptr<TerrainRepresentation> m_terrain;
    mutable std::map<std::string, UnprojectionLookup> m_projector;
};

} // perception
