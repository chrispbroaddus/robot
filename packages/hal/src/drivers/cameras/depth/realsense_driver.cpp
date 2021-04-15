#include "packages/hal/include/drivers/cameras/depth/realsense_driver.h"

#include "glog/logging.h"
#include "librealsense/rs2.hpp"
#include "librealsense/rsutil2.hpp"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <string>
#include <thread>

#include "packages/core/include/chrono.h"

namespace hal {

typedef struct { float x, y, z; } point_t;

constexpr int kDefaultDevice = 0;
constexpr int kBytesPerPixel = 12;

bool depth_to_points(std::vector<point_t>& points, const rs2_intrinsics& depth_intrinsics, const std::vector<uint16_t>& depth_image,
    const float depth_scale) {
    const int width = depth_intrinsics.width;
    const int height = depth_intrinsics.height;
    points.reserve(width * height);
    std::vector<uint16_t>::const_iterator depth_iterator = depth_image.begin();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float pixel[] = { static_cast<float>(x), static_cast<float>(y) };
            float point[3] = { 0, 0, 0 };
            auto depth = static_cast<float>(*depth_iterator++);

            rs2_deproject_pixel_to_point(point, &depth_intrinsics, pixel, depth * depth_scale);
            points.push_back(point_t{ point[0], point[1], point[2] });
        }
    }
    return (points.size() == depth_image.size());
}

class RealsenseDriver::RealsenseDriverImpl {
public:
    RealsenseDriverImpl(RealsenseDriver::Mode mode)
        : m_running(true)
        , m_frameCount(0) {
        m_thread = std::thread(&RealsenseDriverImpl::run, this, mode);
    }

    ~RealsenseDriverImpl() { stop(); }

    void stop() {
        m_running = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void run(const RealsenseDriver::Mode& mode) {
        rs2::context ctx;
        auto devices = ctx.query_devices();
        size_t device_count = devices.size();
        if (!device_count) {
            throw std::runtime_error("Realsense: camera enumeration failed");
        }
        auto device = devices[kDefaultDevice];
        LOG(INFO) << "Using default device!";
        rs2::util::config config;
        switch (mode) {
        case RealsenseDriver::Mode::BEST_QUALITY:
            config.enable_all(rs2::preset::best_quality);
            break;
        default:
            LOG(FATAL) << "Unsupported Mode!";
        };
        m_deviceName = device.get_camera_info(RS2_CAMERA_INFO_DEVICE_NAME);
        m_moduleName = device.get_camera_info(RS2_CAMERA_INFO_MODULE_NAME);
        m_serialNumber = std::stoi(device.get_camera_info(RS2_CAMERA_INFO_DEVICE_SERIAL_NUMBER));

        auto stream = config.open(device);
        auto syncer = device.create_syncer();

        const rs2_intrinsics depth_intrinsics = stream.get_intrinsics(RS2_STREAM_DEPTH);
        // auto depth_units = device.get_option(RS2_OPTION_DEPTH_UNITS);
        // Ian: This query does not work on this unit:
        //  what():  OpCodes do not match! Sent 44 but received -6!
        // Empirically seen to be (mm)
        constexpr auto depth_units = 1.f / 1000.f; // (mm) to (m)

        stream.start(syncer);
        while (m_running) {
            auto frameset = syncer.wait_for_frames();
            for (auto&& frame : frameset) {
                if (frame.get_stream_type() == RS2_STREAM_DEPTH) {
                    auto depth_image_ptr = reinterpret_cast<const uint16_t*>(frame.get_data());
                    auto depth_image
                        = std::vector<uint16_t>(depth_image_ptr, depth_image_ptr + (depth_intrinsics.width * depth_intrinsics.height));
                    CHECK(!depth_image.empty());
                    std::vector<point_t> points;
                    CHECK(depth_to_points(points, depth_intrinsics, depth_image, depth_units));
                    hal::Device* device = new Device();
                    CHECK_NOTNULL(device);
                    device->set_name(m_deviceName);
                    device->set_serialnumber(m_serialNumber);

                    hal::Image* image = new hal::Image();
                    CHECK_NOTNULL(image);
                    image->set_rows(depth_intrinsics.height);
                    image->set_cols(depth_intrinsics.width);

                    int imageWidth = depth_intrinsics.width;
                    int imageHeight = depth_intrinsics.height;

                    image->set_stride(imageWidth * kBytesPerPixel);
                    image->set_data(points.data(), imageWidth * imageHeight * kBytesPerPixel);
                    image->set_type(hal::PB_FLOAT);
                    image->set_format(hal::PB_POINTCLOUD);
                    image->set_stride(3 * imageWidth);
                    const std::chrono::nanoseconds timestamp = core::chrono::gps::wallClockInNanoseconds();

                    // HardwareTimestamp
                    core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();
                    // SystemTimestamp
                    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
                    systemTimestamp->set_nanos(timestamp.count());

                    // Set the CameraSample
                    hal::CameraSample cameraSample;
                    cameraSample.set_allocated_device(device);
                    cameraSample.set_allocated_image(image);
                    cameraSample.set_allocated_systemtimestamp(systemTimestamp);
                    cameraSample.set_allocated_hardwaretimestamp(hardwareTimestamp);
                    cameraSample.set_id(++m_frameCount);
                    {
                        std::lock_guard<std::mutex> lock(m_guard);
                        m_frames.push_front(cameraSample);
                        m_haveData.notify_one();
                    }
                    break;
                }
            }
        }
    }

    bool capture(CameraSample& cameraSample) {
        {
            // Here we rely on signalling (via the condition variable) to notify
            // this call that frame data is available, thus ensuring we are never
            // popping from an empty queue.
            std::unique_lock<std::mutex> lock(m_guard);
            m_haveData.wait(lock, [this] { return !m_frames.empty(); });
        }
        cameraSample = m_frames.front();
        m_frames.clear();
        return true;
    }

    uint64_t serialNumber() const { return m_serialNumber; }

    std::string deviceName() const { return m_deviceName; };

private:
    std::atomic<bool> m_running;
    uint32_t m_frameCount;
    std::string m_deviceName;
    std::string m_moduleName;
    int m_serialNumber;
    std::thread m_thread;
    std::mutex m_guard;
    std::condition_variable m_haveData;
    std::deque<hal::CameraSample> m_frames;
};

RealsenseDriver::RealsenseDriver(Mode mode)
    : m_impl(new RealsenseDriverImpl(mode)) {}

std::string RealsenseDriver::deviceName() const { return m_impl->deviceName(); }

uint64_t RealsenseDriver::serialNumber() const { return m_impl->serialNumber(); }

bool RealsenseDriver::capture(CameraSample& cameraSample) { return m_impl->capture(cameraSample); }

} // hal
