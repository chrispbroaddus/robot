// License: Apache 2.0 See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#include <functional>   // For function

#include "api.h"
#include "context.h"
#include "device.h"
#include "algo.h"
#include "core/debug.h"
#include "core/motion.h"
#include "core/extension.h"
#include "media/record/record_device.h"
#include "core/advanced_mode.h"
#include "align.h"
#include "colorizer.h"
#include "media/playback/playback_device.h"
#include "stream.h"
#include "../include/librealsense2/h/rs_types.h"
#include "pipeline.h"
#include "environment.h"
////////////////////////
// API implementation //
////////////////////////
struct rs2_stream_profile_list
{
    std::vector<std::shared_ptr<stream_profile_interface>> list;
};

struct rs2_sensor
{
    rs2_device parent;
    librealsense::sensor_interface* sensor;
    size_t index;
};


struct rs2_context
{
    ~rs2_context() { ctx->stop(); }
    std::shared_ptr<librealsense::context> ctx;
};

struct rs2_device_hub
{
    std::shared_ptr<librealsense::device_hub> hub;
};

struct rs2_pipeline
{
    std::shared_ptr<librealsense::pipeline> pipe;
};

//struct rs2_intrinsics
//{
//    std::shared_ptr<librealsense::rs2_intrinsics> intrinsics;
//};
//struct rs2_record_device
//{
//    std::shared_ptr<librealsense::record_device> record_device;
//};

//struct rs2_syncer
//{
//    std::shared_ptr<librealsense::sync_interface> syncer;
//};

struct rs2_frame_queue
{
    explicit rs2_frame_queue(int cap)
        : queue(cap)
    {
    }

    single_consumer_queue<librealsense::frame_holder> queue;
};

struct rs2_processing_block
{
    std::shared_ptr<librealsense::processing_block_interface> block;
};

#define NOEXCEPT_RETURN(R, ...) catch(...) { std::ostringstream ss; librealsense::stream_args(ss, #__VA_ARGS__, __VA_ARGS__); rs2_error* e; librealsense::translate_exception(__FUNCTION__, ss.str(), &e); LOG_WARNING(rs2_get_error_message(e)); rs2_free_error(e); return R; }
#define HANDLE_EXCEPTIONS_AND_RETURN(R, ...) catch(...) { std::ostringstream ss; librealsense::stream_args(ss, #__VA_ARGS__, __VA_ARGS__); librealsense::translate_exception(__FUNCTION__, ss.str(), error); return R; }
#define VALIDATE_NOT_NULL(ARG) if(!(ARG)) throw std::runtime_error("null pointer passed for argument \"" #ARG "\"");
#define VALIDATE_ENUM(ARG) if(!librealsense::is_valid(ARG)) { std::ostringstream ss; ss << "invalid enum value for argument \"" #ARG "\""; throw librealsense::invalid_value_exception(ss.str()); }
#define VALIDATE_RANGE(ARG, MIN, MAX) if((ARG) < (MIN) || (ARG) > (MAX)) { std::ostringstream ss; ss << "out of range value for argument \"" #ARG "\""; throw librealsense::invalid_value_exception(ss.str()); }
#define VALIDATE_LE(ARG, MAX) if((ARG) > (MAX)) { std::ostringstream ss; ss << "out of range value for argument \"" #ARG "\""; throw std::runtime_error(ss.str()); }
//#define VALIDATE_NATIVE_STREAM(ARG) VALIDATE_ENUM(ARG); if(ARG >= RS2_STREAM_NATIVE_COUNT) { std::ostringstream ss; ss << "argument \"" #ARG "\" must be a native stream"; throw librealsense::wrong_value_exception(ss.str()); }
struct rs2_sensor_list
{
    rs2_device dev;
};

int major(int version)
{
    return version / 10000;
}
int minor(int version)
{
    return (version % 10000) / 100;
}
int patch(int version)
{
    return (version % 100);
}

std::string api_version_to_string(int version)
{
    if (major(version) == 0) return librealsense::to_string() << version;
    return librealsense::to_string() << major(version) << "." << minor(version) << "." << patch(version);
}

void report_version_mismatch(int runtime, int compiletime)
{
    throw librealsense::invalid_value_exception(librealsense::to_string() << "API version mismatch: librealsense.so was compiled with API version "
                                                                << api_version_to_string(runtime) << " but the application was compiled with "
                                                                << api_version_to_string(compiletime) << "! Make sure correct version of the library is installed (make install)");
}

void verify_version_compatibility(int api_version)
{
    rs2_error* error = nullptr;
    auto runtime_api_version = rs2_get_api_version(&error);
    if (error)
        throw librealsense::invalid_value_exception(rs2_get_error_message(error));

    if ((runtime_api_version < 10) || (api_version < 10))
    {
        // when dealing with version < 1.0.0 that were still using single number for API version, require exact match
        if (api_version != runtime_api_version)
            report_version_mismatch(runtime_api_version, api_version);
    }
    else if ((major(runtime_api_version) == 1 && minor(runtime_api_version) <= 9)
        || (major(api_version) == 1 && minor(api_version) <= 9))
    {
        // when dealing with version < 1.10.0, API breaking changes are still possible without minor version change, require exact match
        if (api_version != runtime_api_version)
            report_version_mismatch(runtime_api_version, api_version);
    }
    else
    {
        // starting with 1.10.0, versions with same patch are compatible
        if ((major(api_version) != major(runtime_api_version))
            || (minor(api_version) != minor(runtime_api_version)))
            report_version_mismatch(runtime_api_version, api_version);
    }
}


void notifications_proccessor::raise_notification(const notification n)
{
    _dispatcher.invoke([this, n](dispatcher::cancellable_timer ct)
    {
        std::lock_guard<std::mutex> lock(_callback_mutex);
        rs2_notification noti(&n);
        if (_callback)_callback->on_notification(&noti);
        else
        {
#ifdef DEBUG

#endif // !DEBUG

        }
    });
}

rs2_context* rs2_create_context(int api_version, rs2_error** error) try
{
    verify_version_compatibility(api_version);

    return new rs2_context{ std::make_shared<librealsense::context>(librealsense::backend_type::standard) };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, api_version)

void rs2_delete_context(rs2_context* context) try
{
    VALIDATE_NOT_NULL(context);
    delete context;
}
NOEXCEPT_RETURN(, context)

rs2_device_hub* rs2_create_device_hub(const rs2_context* context, rs2_error** error) try
{
    return new rs2_device_hub{ std::make_shared<librealsense::device_hub>(context->ctx) };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, context)

void rs2_delete_device_hub(const rs2_device_hub* hub) try
{
    VALIDATE_NOT_NULL(hub);
    delete hub;
}
NOEXCEPT_RETURN(, hub)

rs2_device* rs2_device_hub_wait_for_device(rs2_context* ctx, const rs2_device_hub* hub, rs2_error** error) try
{
    VALIDATE_NOT_NULL(hub);
    VALIDATE_NOT_NULL(ctx);
    return new rs2_device{ ctx->ctx, /*TODO: how does this affect the new device*/ nullptr, hub->hub->wait_for_device() };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, hub, ctx)

int rs2_device_hub_is_device_connected(const rs2_device_hub* hub, const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(hub);
    VALIDATE_NOT_NULL(device);
    auto res = hub->hub->is_connected(*device->device);
    return res ? 1 : 0;
}
HANDLE_EXCEPTIONS_AND_RETURN(0, hub, device)

rs2_device_list* rs2_query_devices(const rs2_context* context, rs2_error** error) try
{
    VALIDATE_NOT_NULL(context);

    std::vector<rs2_device_info> results;
    for (auto&& dev_info : context->ctx->query_devices())
    {
        try
        {
            rs2_device_info d{ context->ctx, dev_info };
            results.push_back(d);
        }
        catch (...)
        {
            LOG_WARNING("Could not open device!");
        }
    }

    return new rs2_device_list{ context->ctx, results };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, context)

rs2_sensor_list* rs2_query_sensors(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);

    std::vector<rs2_device_info> results;
    try
    {
        auto dev = device->device;
        for (unsigned int i = 0; i < dev->get_sensors_count(); i++)
        {
            rs2_device_info d{ device->ctx, device->info };
            results.push_back(d);
        }
    }
    catch (...)
    {
        LOG_WARNING("Could not open device!");
    }

    return new rs2_sensor_list{ *device };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, device)

int rs2_get_device_count(const rs2_device_list* list, rs2_error** error) try
{
    if (list == nullptr)
        return 0;
    return static_cast<int>(list->list.size());
}
HANDLE_EXCEPTIONS_AND_RETURN(0, list)

int rs2_get_sensors_count(const rs2_sensor_list* list, rs2_error** error) try
{
    if (list == nullptr)
        return 0;
    return static_cast<int>(list->dev.device->get_sensors_count());
}
HANDLE_EXCEPTIONS_AND_RETURN(0, list)

void rs2_delete_device_list(rs2_device_list* list) try
{
    VALIDATE_NOT_NULL(list);
    delete list;
}
NOEXCEPT_RETURN(, list)

void rs2_delete_sensor_list(rs2_sensor_list* list) try
{
    VALIDATE_NOT_NULL(list);
    delete list;
}
NOEXCEPT_RETURN(, list)

rs2_device* rs2_create_device(const rs2_device_list* info_list, int index, rs2_error** error) try
{
    VALIDATE_NOT_NULL(info_list);
    VALIDATE_RANGE(index, 0, (int)info_list->list.size() - 1);

    return new rs2_device{ info_list->ctx,
                          info_list->list[index].info,
                          info_list->list[index].info->create_device()
    };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, info_list, index)

void rs2_delete_device(rs2_device* device) try
{
    VALIDATE_NOT_NULL(device);
    delete device;
}
NOEXCEPT_RETURN(, device)

rs2_sensor* rs2_create_sensor(const rs2_sensor_list* list, int index, rs2_error** error) try
{
    VALIDATE_NOT_NULL(list);
    VALIDATE_RANGE(index, 0, (int)list->dev.device->get_sensors_count() - 1);

    return new rs2_sensor{
            list->dev,
            &list->dev.device->get_sensor(index),
            (size_t)index
    };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, list, index)

void rs2_delete_sensor(rs2_sensor* device) try
{
    VALIDATE_NOT_NULL(device);
    delete device;
}
NOEXCEPT_RETURN(, device)

rs2_stream_profile_list* rs2_get_stream_profiles(rs2_sensor* sensor, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    return new rs2_stream_profile_list{ sensor->sensor->get_stream_profiles() };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, sensor)

const rs2_stream_profile* rs2_get_stream_profile(const rs2_stream_profile_list* list, int index, rs2_error** error) try
{
    VALIDATE_NOT_NULL(list);
    VALIDATE_RANGE(index, 0, (int)list->list.size() - 1);

    return list->list[index]->get_c_wrapper();
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, list, index)

int rs2_get_stream_profiles_count(const rs2_stream_profile_list* list, rs2_error** error) try
{
    VALIDATE_NOT_NULL(list);
    return static_cast<int>(list->list.size());
}
HANDLE_EXCEPTIONS_AND_RETURN(0, list)

void rs2_delete_stream_profiles_list(rs2_stream_profile_list* list) try
{
    VALIDATE_NOT_NULL(list);
    delete list;
}
NOEXCEPT_RETURN(, list)

void rs2_get_video_stream_intrinsics(const rs2_stream_profile* from, rs2_intrinsics* intr, rs2_error** error) try
{
    VALIDATE_NOT_NULL(from);
    VALIDATE_NOT_NULL(intr);

    auto vid = VALIDATE_INTERFACE(from->profile, librealsense::video_stream_profile_interface);

    *intr = vid->get_intrinsics();
}
HANDLE_EXCEPTIONS_AND_RETURN(, from, intr)

void rs2_get_video_stream_resolution(const rs2_stream_profile* from, int* width, int* height, rs2_error** error) try
{
    VALIDATE_NOT_NULL(from);

    auto vid = VALIDATE_INTERFACE(from->profile, librealsense::video_stream_profile_interface);

    if (width)  *width = vid->get_width();
    if (height) *height = vid->get_height();
}
HANDLE_EXCEPTIONS_AND_RETURN(, from, width, height)

int rs2_get_stream_profile_size(const rs2_stream_profile* profile, rs2_error** error) try
{
    VALIDATE_NOT_NULL(profile);
    return static_cast<int>(profile->profile->get_size());
}
HANDLE_EXCEPTIONS_AND_RETURN(0, profile)

int rs2_is_stream_profile_default(const rs2_stream_profile* profile, rs2_error** error) try
{
    VALIDATE_NOT_NULL(profile);
    return profile->profile->is_default() ? 1 : 0;
}
HANDLE_EXCEPTIONS_AND_RETURN(0, profile)

void rs2_get_stream_profile_data(const rs2_stream_profile* mode, rs2_stream* stream, rs2_format* format, int* index, int* unique_id, int* framerate, rs2_error** error) try
{
    VALIDATE_NOT_NULL(mode);
    VALIDATE_NOT_NULL(stream);
    VALIDATE_NOT_NULL(format);
    VALIDATE_NOT_NULL(index);
    VALIDATE_NOT_NULL(unique_id);

    *framerate = mode->profile->get_framerate();
    *format = mode->profile->get_format();
    *index = mode->profile->get_stream_index();
    *stream = mode->profile->get_stream_type();
    *unique_id = mode->profile->get_unique_id();
}
HANDLE_EXCEPTIONS_AND_RETURN(, mode, stream, format, index, framerate)

void rs2_set_stream_profile_data(rs2_stream_profile* mode, rs2_stream stream, int index, rs2_format format, rs2_error** error) try
{
    VALIDATE_NOT_NULL(mode);
    VALIDATE_ENUM(stream);
    VALIDATE_ENUM(format);

    mode->profile->set_format(format);
    mode->profile->set_stream_type(stream);
    mode->profile->set_stream_index(index);
}
HANDLE_EXCEPTIONS_AND_RETURN(, mode, stream, format)

void rs2_delete_stream_profile(rs2_stream_profile* p) try
{
    VALIDATE_NOT_NULL(p);
    delete p;
}
NOEXCEPT_RETURN(, p)

rs2_stream_profile* rs2_clone_stream_profile(const rs2_stream_profile* mode, rs2_stream stream, int stream_idx, rs2_format fmt, rs2_error** error) try
{
    VALIDATE_NOT_NULL(mode);
    VALIDATE_ENUM(stream);
    VALIDATE_ENUM(fmt);

    auto sp = mode->profile->clone();
    sp->set_stream_type(stream);
    sp->set_stream_index(stream_idx);
    sp->set_format(fmt);

    return new rs2_stream_profile{ sp.get(), sp };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, mode, stream, stream_idx, fmt)

const rs2_raw_data_buffer* rs2_send_and_receive_raw_data(rs2_device* device, void* raw_data_to_send, unsigned size_of_raw_data_to_send, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);

    auto debug_interface = VALIDATE_INTERFACE(device->device, librealsense::debug_interface);

    auto raw_data_buffer = static_cast<uint8_t*>(raw_data_to_send);
    std::vector<uint8_t> buffer_to_send(raw_data_buffer, raw_data_buffer + size_of_raw_data_to_send);
    auto ret_data = debug_interface->send_receive_raw_data(buffer_to_send);
    return new rs2_raw_data_buffer{ ret_data };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, device)

const unsigned char* rs2_get_raw_data(const rs2_raw_data_buffer* buffer, rs2_error** error) try
{
    VALIDATE_NOT_NULL(buffer);
    return buffer->buffer.data();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, buffer)

int rs2_get_raw_data_size(const rs2_raw_data_buffer* buffer, rs2_error** error) try
{
    VALIDATE_NOT_NULL(buffer);
    return static_cast<int>(buffer->buffer.size());
}
HANDLE_EXCEPTIONS_AND_RETURN(0, buffer)

void rs2_delete_raw_data(const rs2_raw_data_buffer* buffer) try
{
    VALIDATE_NOT_NULL(buffer);
    delete buffer;
}
NOEXCEPT_RETURN(, buffer)

void rs2_open(rs2_sensor* sensor, const rs2_stream_profile* profile, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(profile);

    std::vector<std::shared_ptr<stream_profile_interface>> request;
    request.push_back(std::dynamic_pointer_cast<stream_profile_interface>(profile->profile->shared_from_this()));
    sensor->sensor->open(request);
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, profile)

void rs2_open_multiple(rs2_sensor* sensor,
    const rs2_stream_profile** profiles, int count, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(profiles);

    std::vector<std::shared_ptr<stream_profile_interface>> request;
    for (auto i = 0; i < count; i++)
    {
        request.push_back(std::dynamic_pointer_cast<stream_profile_interface>(profiles[i]->profile->shared_from_this()));
    }
    sensor->sensor->open(request);
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, profiles, count)

void rs2_close(const rs2_sensor* sensor, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    sensor->sensor->close();
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor)

int rs2_is_option_read_only(const rs2_sensor* sensor, rs2_option option, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(option);
    return sensor->sensor->get_option(option).is_read_only();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, sensor, option)

float rs2_get_option(const rs2_sensor* sensor, rs2_option option, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(option);
    return sensor->sensor->get_option(option).query();
}
HANDLE_EXCEPTIONS_AND_RETURN(0.0f, sensor, option)

void rs2_set_option(const rs2_sensor* sensor, rs2_option option, float value, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(option);
    sensor->sensor->get_option(option).set(value);
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, option, value)

int rs2_supports_option(const rs2_sensor* sensor, rs2_option option, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(option);
    return sensor->sensor->supports_option(option);
}
HANDLE_EXCEPTIONS_AND_RETURN(0, sensor, option)

void rs2_get_option_range(const rs2_sensor* sensor, rs2_option option,
    float* min, float* max, float* step, float* def, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(option);
    VALIDATE_NOT_NULL(min);
    VALIDATE_NOT_NULL(max);
    VALIDATE_NOT_NULL(step);
    VALIDATE_NOT_NULL(def);
    auto range = sensor->sensor->get_option(option).get_range();
    *min = range.min;
    *max = range.max;
    *def = range.def;
    *step = range.step;
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, option, min, max, step, def)

const char* rs2_get_device_info(const rs2_device* dev, rs2_camera_info info, rs2_error** error) try
{
    VALIDATE_NOT_NULL(dev);
    VALIDATE_ENUM(info);
    if (dev->device->supports_info(info))
    {
        return dev->device->get_info(info).c_str();
    }
    throw librealsense::invalid_value_exception(librealsense::to_string() << "info " << rs2_camera_info_to_string(info) << " not supported by the device!");
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, dev, info)

int rs2_supports_device_info(const rs2_device* dev, rs2_camera_info info, rs2_error** error) try
{
    VALIDATE_NOT_NULL(dev);
    VALIDATE_ENUM(info);
    return dev->device->supports_info(info);
}
HANDLE_EXCEPTIONS_AND_RETURN(false, dev, info)

const char* rs2_get_sensor_info(const rs2_sensor* sensor, rs2_camera_info info, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(info);
    if (sensor->sensor->supports_info(info))
    {
        return sensor->sensor->get_info(info).c_str();
    }
    throw librealsense::invalid_value_exception(librealsense::to_string() << "info " << rs2_camera_info_to_string(info) << " not supported by the sensor!");
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, sensor, info)

int rs2_supports_sensor_info(const rs2_sensor* sensor, rs2_camera_info info, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(info);
    return sensor->sensor->supports_info(info);
}
HANDLE_EXCEPTIONS_AND_RETURN(false, sensor, info)

void rs2_start(const rs2_sensor* sensor, rs2_frame_callback_ptr on_frame, void* user, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(on_frame);
    librealsense::frame_callback_ptr callback(
        new librealsense::frame_callback(on_frame, user));
    sensor->sensor->start(move(callback));
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, on_frame, user)

void rs2_start_queue(const rs2_sensor* sensor, rs2_frame_queue* queue, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(queue);
    librealsense::frame_callback_ptr callback(
        new librealsense::frame_callback(rs2_enqueue_frame, queue));
    sensor->sensor->start(move(callback));
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, queue)

void rs2_set_notifications_callback(const rs2_sensor* sensor, rs2_notification_callback_ptr on_notification, void* user, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(on_notification);
    librealsense::notifications_callback_ptr callback(
        new librealsense::notifications_callback(on_notification, user),
        [](rs2_notifications_callback* p) { delete p; });
    sensor->sensor->register_notifications_callback(std::move(callback));
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, on_notification, user)

void rs2_set_devices_changed_callback(const rs2_context* context, rs2_devices_changed_callback_ptr callback, void* user, rs2_error** error) try
{
    VALIDATE_NOT_NULL(context);
    VALIDATE_NOT_NULL(callback);
    librealsense::devices_changed_callback_ptr cb(
        new librealsense::devices_changed_callback(callback, user),
        [](rs2_devices_changed_callback* p) { delete p; });
    context->ctx->set_devices_changed_callback(std::move(cb));
}
HANDLE_EXCEPTIONS_AND_RETURN(, context, callback, user)

void rs2_start_cpp(const rs2_sensor* sensor, rs2_frame_callback* callback, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(callback);
    sensor->sensor->start({ callback, [](rs2_frame_callback* p) { p->release(); } });
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, callback)

void rs2_set_notifications_callback_cpp(const rs2_sensor* sensor, rs2_notifications_callback* callback, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(callback);
    sensor->sensor->register_notifications_callback({ callback, [](rs2_notifications_callback* p) { p->release(); } });
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, callback)

void rs2_set_devices_changed_callback_cpp(rs2_context* context, rs2_devices_changed_callback* callback, rs2_error** error) try
{
    VALIDATE_NOT_NULL(context);
    VALIDATE_NOT_NULL(callback);
    context->ctx->set_devices_changed_callback({ callback, [](rs2_devices_changed_callback* p) { p->release(); } });
}
HANDLE_EXCEPTIONS_AND_RETURN(, context, callback)

void rs2_stop(const rs2_sensor* sensor, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    sensor->sensor->stop();
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor)

int rs2_supports_frame_metadata(const rs2_frame* frame, rs2_frame_metadata_value frame_metadata, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);
    VALIDATE_ENUM(frame_metadata);
    return ((frame_interface*)frame)->supports_frame_metadata(frame_metadata);
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame, frame_metadata)

rs2_metadata_type rs2_get_frame_metadata(const rs2_frame* frame, rs2_frame_metadata_value frame_metadata, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);
    VALIDATE_ENUM(frame_metadata);
    return ((frame_interface*)frame)->get_frame_metadata(frame_metadata);
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame, frame_metadata)

const char* rs2_get_notification_description(rs2_notification* notification, rs2_error** error)try
{
    VALIDATE_NOT_NULL(notification);
    return notification->_notification->description.c_str();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, notification)

rs2_time_t rs2_get_notification_timestamp(rs2_notification* notification, rs2_error** error)try
{
    VALIDATE_NOT_NULL(notification);
    return notification->_notification->timestamp;
}
HANDLE_EXCEPTIONS_AND_RETURN(0, notification)

rs2_log_severity rs2_get_notification_severity(rs2_notification* notification, rs2_error** error)try
{
    VALIDATE_NOT_NULL(notification);
    return (rs2_log_severity)notification->_notification->severity;
}
HANDLE_EXCEPTIONS_AND_RETURN(RS2_LOG_SEVERITY_NONE, notification)

rs2_notification_category rs2_get_notification_category(rs2_notification* notification, rs2_error** error)try
{
    VALIDATE_NOT_NULL(notification);
    return (rs2_notification_category)notification->_notification->category;
}
HANDLE_EXCEPTIONS_AND_RETURN(RS2_NOTIFICATION_CATEGORY_UNKNOWN_ERROR, notification)



int rs2_device_list_contains(const rs2_device_list* info_list, const rs2_device* device, rs2_error** error)try
{
    VALIDATE_NOT_NULL(info_list);
    VALIDATE_NOT_NULL(device);

    for (auto info : info_list->list)
    {
        // TODO: This is incapable of detecting playback devices
        // Need to extend, if playback, compare filename or something
        if (device->info && device->info->get_device_data() == info.info->get_device_data())
        {
            return 1;
        }
    }
    return 0;
}
HANDLE_EXCEPTIONS_AND_RETURN(false, info_list, device)

rs2_time_t rs2_get_frame_timestamp(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    return ((frame_interface*)frame_ref)->get_frame_timestamp();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame_ref)

rs2_timestamp_domain rs2_get_frame_timestamp_domain(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    return ((frame_interface*)frame_ref)->get_frame_timestamp_domain();
}
HANDLE_EXCEPTIONS_AND_RETURN(RS2_TIMESTAMP_DOMAIN_COUNT, frame_ref)

const void* rs2_get_frame_data(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    return ((frame_interface*)frame_ref)->get_frame_data();
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, frame_ref)

int rs2_get_frame_width(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    auto vf = VALIDATE_INTERFACE(((frame_interface*)frame_ref), librealsense::video_frame);
    return vf->get_width();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame_ref)

int rs2_get_frame_height(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    auto vf = VALIDATE_INTERFACE(((frame_interface*)frame_ref), librealsense::video_frame);
    return vf->get_height();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame_ref)

int rs2_get_frame_stride_in_bytes(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    auto vf = VALIDATE_INTERFACE(((frame_interface*)frame_ref), librealsense::video_frame);
    return vf->get_stride();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame_ref)

const rs2_stream_profile* rs2_get_frame_stream_profile(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    return ((frame_interface*)frame_ref)->get_stream()->get_c_wrapper();
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, frame_ref)

int rs2_get_frame_bits_per_pixel(const rs2_frame* frame_ref, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    auto vf = VALIDATE_INTERFACE(((frame_interface*)frame_ref), librealsense::video_frame);
    return vf->get_bpp();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame_ref)

unsigned long long rs2_get_frame_number(const rs2_frame* frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);
    return ((frame_interface*)frame)->get_frame_number();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame)

void rs2_release_frame(rs2_frame* frame) try
{
    VALIDATE_NOT_NULL(frame);
    ((frame_interface*)frame)->release();
}
NOEXCEPT_RETURN(, frame)

const char* rs2_get_option_description(const rs2_sensor* sensor, rs2_option option, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(option);
    return sensor->sensor->get_option(option).get_description();
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, sensor, option)

void rs2_frame_add_ref(rs2_frame* frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);
    ((frame_interface*)frame)->acquire();
}
HANDLE_EXCEPTIONS_AND_RETURN(, frame)

const char* rs2_get_option_value_description(const rs2_sensor* sensor, rs2_option option, float value, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(option);
    return sensor->sensor->get_option(option).get_value_description(value);
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, sensor, option, value)

rs2_frame_queue* rs2_create_frame_queue(int capacity, rs2_error** error) try
{
    return new rs2_frame_queue(capacity);
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, capacity)

void rs2_delete_frame_queue(rs2_frame_queue* queue) try
{
    VALIDATE_NOT_NULL(queue);
    delete queue;
}
NOEXCEPT_RETURN(, queue)

rs2_frame* rs2_wait_for_frame(rs2_frame_queue* queue, unsigned int timeout_ms, rs2_error** error) try
{
    VALIDATE_NOT_NULL(queue);
    librealsense::frame_holder fh;
    if (!queue->queue.dequeue(&fh, timeout_ms))
    {
        throw std::runtime_error("Frame did not arrive in time!");
    }

    frame_interface* result = nullptr;
    std::swap(result, fh.frame);
    return (rs2_frame*)result;
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, queue)

int rs2_poll_for_frame(rs2_frame_queue* queue, rs2_frame** output_frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(queue);
    VALIDATE_NOT_NULL(output_frame);
    librealsense::frame_holder fh;
    if (queue->queue.try_dequeue(&fh))
    {
        frame_interface* result = nullptr;
        std::swap(result, fh.frame);
        *output_frame = (rs2_frame*)result;
        return true;
    }

    return false;
}
HANDLE_EXCEPTIONS_AND_RETURN(0, queue, output_frame)

void rs2_enqueue_frame(rs2_frame* frame, void* queue) try
{
    VALIDATE_NOT_NULL(frame);
    VALIDATE_NOT_NULL(queue);
    auto q = reinterpret_cast<rs2_frame_queue*>(queue);
    librealsense::frame_holder fh;
    fh.frame = (frame_interface*)frame;
    q->queue.enqueue(std::move(fh));
}
NOEXCEPT_RETURN(, frame, queue)

void rs2_flush_queue(rs2_frame_queue* queue, rs2_error** error) try
{
    VALIDATE_NOT_NULL(queue);
    queue->queue.clear();
}
HANDLE_EXCEPTIONS_AND_RETURN(, queue)

void rs2_get_extrinsics(const rs2_stream_profile* from,
    const rs2_stream_profile* to,
    rs2_extrinsics* extrin, rs2_error** error) try
{
    VALIDATE_NOT_NULL(from);
    VALIDATE_NOT_NULL(to);
    VALIDATE_NOT_NULL(extrin);

    if (!environment::get_instance().get_extrinsics_graph().try_fetch_extrinsics(*from->profile, *to->profile, extrin))
    {
        throw not_implemented_exception("Requested extrinsics are not available!");
    }
}
HANDLE_EXCEPTIONS_AND_RETURN(, from, to, extrin)

void rs2_get_motion_intrinsics(const rs2_sensor* sensor, rs2_stream stream, rs2_motion_device_intrinsic* intrinsics, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(intrinsics);
    VALIDATE_ENUM(stream);

    auto motion = VALIDATE_INTERFACE(sensor->sensor, librealsense::motion_sensor_interface);
    *intrinsics = motion->get_motion_intrinsics(stream);
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, stream, intrinsics)

void rs2_hardware_reset(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    device->device->hardware_reset();
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

// Verify  and provide API version encoded as integer value
int rs2_get_api_version(rs2_error** error) try
{
    // Each component type is within [0-99] range
    VALIDATE_RANGE(RS2_API_MAJOR_VERSION, 0, 99);
    VALIDATE_RANGE(RS2_API_MINOR_VERSION, 0, 99);
    VALIDATE_RANGE(RS2_API_PATCH_VERSION, 0, 99);
    return RS2_API_VERSION;
}
HANDLE_EXCEPTIONS_AND_RETURN(0, RS2_API_MAJOR_VERSION, RS2_API_MINOR_VERSION, RS2_API_PATCH_VERSION)

rs2_context* rs2_create_recording_context(int api_version, const char* filename, const char* section, rs2_recording_mode mode, rs2_error** error) try
{
    VALIDATE_NOT_NULL(filename);
    VALIDATE_NOT_NULL(section);
    verify_version_compatibility(api_version);

    return new rs2_context{ std::make_shared<librealsense::context>(librealsense::backend_type::record, filename, section, mode) };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, api_version, filename, section, mode)

rs2_context* rs2_create_mock_context(int api_version, const char* filename, const char* section, rs2_error** error) try
{
    VALIDATE_NOT_NULL(filename);
    VALIDATE_NOT_NULL(section);
    verify_version_compatibility(api_version);

    return new rs2_context{ std::make_shared<librealsense::context>(librealsense::backend_type::playback, filename, section) };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, api_version, filename, section)

void rs2_set_region_of_interest(const rs2_sensor* sensor, int min_x, int min_y, int max_x, int max_y, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);

    VALIDATE_LE(min_x, max_x);
    VALIDATE_LE(min_y, max_y);
    VALIDATE_LE(0, min_x);
    VALIDATE_LE(0, min_y);

    auto roi = VALIDATE_INTERFACE(sensor->sensor, librealsense::roi_sensor_interface);
    roi->get_roi_method().set({ min_x, min_y, max_x, max_y });
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, min_x, min_y, max_x, max_y)

void rs2_get_region_of_interest(const rs2_sensor* sensor, int* min_x, int* min_y, int* max_x, int* max_y, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_NOT_NULL(min_x);
    VALIDATE_NOT_NULL(min_y);
    VALIDATE_NOT_NULL(max_x);
    VALIDATE_NOT_NULL(max_y);

    auto roi = VALIDATE_INTERFACE(sensor->sensor, librealsense::roi_sensor_interface);
    auto rect = roi->get_roi_method().get();

    *min_x = rect.min_x;
    *min_y = rect.min_y;
    *max_x = rect.max_x;
    *max_y = rect.max_y;
}
HANDLE_EXCEPTIONS_AND_RETURN(, sensor, min_x, min_y, max_x, max_y)

//rs2_syncer* rs2_create_syncer(rs2_error** error) try
//{
//    return new rs2_syncer{ std::make_shared<librealsense::syncer>() };
//}
//catch (...) { librealsense::translate_exception(__FUNCTION__, "", error); return nullptr; }

//void rs2_start_syncer(const rs2_sensor* sensor, rs2_syncer* syncer, rs2_error** error) try
//{
//    VALIDATE_NOT_NULL(sensor);
//    VALIDATE_NOT_NULL(syncer);
//    librealsense::frame_callback_ptr callback(
//        new librealsense::frame_callback(rs2_sync_frame, syncer));
//    sensor->sensor->start(move(callback));
//}
//HANDLE_EXCEPTIONS_AND_RETURN(, sensor, syncer)
//
//void rs2_wait_for_frames(rs2_syncer* syncer, unsigned int timeout_ms, rs2_frame** output_array, rs2_error** error) try
//{
//    VALIDATE_NOT_NULL(syncer);
//    VALIDATE_NOT_NULL(output_array);
//    auto res = syncer->syncer->wait_for_frames(timeout_ms);
//    for (uint32_t i = 0; i < RS2_STREAM_COUNT; i++)
//    {
//        output_array[i] = nullptr;
//    }
//    for (auto&& holder : res)
//    {
//        output_array[holder.frame->get()->get_stream_type()] = holder.frame;
//        holder.frame = nullptr;
//    }
//}
//HANDLE_EXCEPTIONS_AND_RETURN(, syncer, timeout_ms, output_array)
//
//int rs2_poll_for_frames(rs2_syncer* syncer, rs2_frame** output_array, rs2_error** error) try
//{
//    VALIDATE_NOT_NULL(syncer);
//    VALIDATE_NOT_NULL(output_array);
//    librealsense::frameset res;
//    if (syncer->syncer->poll_for_frames(res))
//    {
//        for (uint32_t i = 0; i < RS2_STREAM_COUNT; i++)
//        {
//            output_array[i] = nullptr;
//        }
//        for (auto&& holder : res)
//        {
//            output_array[holder.frame->get()->get_stream_type()] = holder.frame;
//            holder.frame = nullptr;
//        }
//        return 1;
//    }
//    return 0;
//}
//HANDLE_EXCEPTIONS_AND_RETURN(0, syncer, output_array)
//
//void rs2_sync_frame(rs2_frame* frame, void* syncer) try
//{
//    VALIDATE_NOT_NULL(frame);
//    VALIDATE_NOT_NULL(syncer);
//    ((rs2_syncer*)syncer)->syncer->dispatch_frame(frame);
//}
//NOEXCEPT_RETURN(, frame, syncer)
//
//void rs2_delete_syncer(rs2_syncer* syncer) try
//{
//    VALIDATE_NOT_NULL(syncer);
//    delete syncer;
//}
//NOEXCEPT_RETURN(, syncer)

void rs2_free_error(rs2_error* error) { if (error) delete error; }
const char* rs2_get_failed_function(const rs2_error* error) { return error ? error->function : nullptr; }
const char* rs2_get_failed_args(const rs2_error* error) { return error ? error->args.c_str() : nullptr; }
const char* rs2_get_error_message(const rs2_error* error) { return error ? error->message.c_str() : nullptr; }
rs2_exception_type rs2_get_librealsense_exception_type(const rs2_error* error) { return error ? error->exception_type : RS2_EXCEPTION_TYPE_UNKNOWN; }

const char* rs2_stream_to_string(rs2_stream stream) { return librealsense::get_string(stream); }
const char* rs2_format_to_string(rs2_format format) { return librealsense::get_string(format); }
const char* rs2_distortion_to_string(rs2_distortion distortion) { return librealsense::get_string(distortion); }
const char* rs2_option_to_string(rs2_option option) { return librealsense::get_string(option); }
const char* rs2_camera_info_to_string(rs2_camera_info info) { return librealsense::get_string(info); }

const char* rs2_frame_metadata_to_string(rs2_frame_metadata_value metadata) { return librealsense::get_string(metadata); }
const char* rs2_timestamp_domain_to_string(rs2_timestamp_domain info){ return librealsense::get_string(info); }

const char* rs2_notification_category_to_string(rs2_notification_category category) { return librealsense::get_string(category); }

const char* rs2_sr300_visual_preset_to_string(rs2_sr300_visual_preset preset) { return librealsense::get_string(preset); }
const char* rs2_log_severity_to_string(rs2_log_severity severity) { return librealsense::get_string(severity); }
const char* rs2_exception_type_to_string(rs2_exception_type type) { return librealsense::get_string(type); }
const char* rs2_extension_type_to_string(rs2_extension type) { return librealsense::get_string(type); }
const char* rs2_playback_status_to_string(rs2_playback_status status) { return librealsense::get_string(status); }

void rs2_log_to_console(rs2_log_severity min_severity, rs2_error** error) try
{
    librealsense::log_to_console(min_severity);
}
HANDLE_EXCEPTIONS_AND_RETURN(, min_severity)

void rs2_log_to_file(rs2_log_severity min_severity, const char* file_path, rs2_error** error) try
{
    librealsense::log_to_file(min_severity, file_path);
}
HANDLE_EXCEPTIONS_AND_RETURN(, min_severity, file_path)

int rs2_is_sensor_extendable_to(const rs2_sensor* sensor, rs2_extension extension_type, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    VALIDATE_ENUM(extension_type);
    switch (extension_type)
    {
        case RS2_EXTENSION_DEBUG :         return VALIDATE_INTERFACE_NO_THROW(sensor->sensor, librealsense::debug_interface) != nullptr;
        case RS2_EXTENSION_INFO :          return VALIDATE_INTERFACE_NO_THROW(sensor->sensor, librealsense::info_interface) != nullptr;
        case RS2_EXTENSION_MOTION :        return VALIDATE_INTERFACE_NO_THROW(sensor->sensor, librealsense::motion_sensor_interface) != nullptr;
        case RS2_EXTENSION_OPTIONS :       return VALIDATE_INTERFACE_NO_THROW(sensor->sensor, librealsense::options_interface) != nullptr;
        case RS2_EXTENSION_VIDEO :         return VALIDATE_INTERFACE_NO_THROW(sensor->sensor, librealsense::video_sensor_interface) != nullptr;
        case RS2_EXTENSION_ROI :           return VALIDATE_INTERFACE_NO_THROW(sensor->sensor, librealsense::roi_sensor_interface) != nullptr;
        case RS2_EXTENSION_DEPTH_SENSOR :  return VALIDATE_INTERFACE_NO_THROW(sensor->sensor, librealsense::depth_sensor) != nullptr;
        default:
            return 0;
    }
}
HANDLE_EXCEPTIONS_AND_RETURN(0, sensor, extension_type)

int rs2_is_device_extendable_to(const rs2_device* dev, rs2_extension extension, rs2_error** error) try
{
    VALIDATE_NOT_NULL(dev);
    VALIDATE_ENUM(extension);
    switch (extension)
    {
        case RS2_EXTENSION_DEBUG         : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::debug_interface)             != nullptr;
        case RS2_EXTENSION_INFO          : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::info_interface)              != nullptr;
        case RS2_EXTENSION_MOTION        : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::motion_sensor_interface)     != nullptr;
        case RS2_EXTENSION_OPTIONS       : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::options_interface)           != nullptr;
        case RS2_EXTENSION_VIDEO         : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::video_sensor_interface)      != nullptr;
        case RS2_EXTENSION_ROI           : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::roi_sensor_interface)        != nullptr;
        case RS2_EXTENSION_DEPTH_SENSOR  : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::depth_sensor)                != nullptr;
        case RS2_EXTENSION_ADVANCED_MODE : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::ds5_advanced_mode_interface) != nullptr;
        case RS2_EXTENSION_RECORD        : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::record_device)               != nullptr;
        case RS2_EXTENSION_PLAYBACK      : return VALIDATE_INTERFACE_NO_THROW(dev->device, librealsense::playback_device)             != nullptr;
        default:
            return 0;
    }
}
HANDLE_EXCEPTIONS_AND_RETURN(0, dev, extension)


int rs2_is_frame_extendable_to(const rs2_frame* f, rs2_extension extension_type, rs2_error** error) try
{
    VALIDATE_NOT_NULL(f);
    VALIDATE_ENUM(extension_type);
    switch (extension_type)
    {
        case RS2_EXTENSION_VIDEO_FRAME :     return VALIDATE_INTERFACE_NO_THROW((frame_interface*)f, librealsense::video_frame) != nullptr;
        case RS2_EXTENSION_COMPOSITE_FRAME : return VALIDATE_INTERFACE_NO_THROW((frame_interface*)f, librealsense::composite_frame) != nullptr;
        case RS2_EXTENSION_POINTS :          return VALIDATE_INTERFACE_NO_THROW((frame_interface*)f, librealsense::points) != nullptr;
        case RS2_EXTENSION_DEPTH_FRAME:      return VALIDATE_INTERFACE_NO_THROW((frame_interface*)f, librealsense::depth_frame) != nullptr;
        //case RS2_EXTENSION_MOTION_FRAME :  return VALIDATE_INTERFACE_NO_THROW((frame_interface*)f, librealsense::motion_frame) != nullptr;

    default:
        return 0;
    }
}
HANDLE_EXCEPTIONS_AND_RETURN(0, f, extension_type)

int rs2_stream_profile_is(const rs2_stream_profile* f, rs2_extension extension_type, rs2_error** error) try
{
    VALIDATE_NOT_NULL(f);
    VALIDATE_ENUM(extension_type);
    switch (extension_type)
    {
    case RS2_EXTENSION_VIDEO_PROFILE:   return VALIDATE_INTERFACE_NO_THROW(f->profile, librealsense::video_stream_profile_interface) != nullptr;
    default:
        return 0;
    }
}
HANDLE_EXCEPTIONS_AND_RETURN(0, f, extension_type)

unsigned long long int rs2_playback_get_duration(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    return playback->get_duration();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, device)

void rs2_playback_seek(const rs2_device* device, long long int time, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    VALIDATE_LE(0, time);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    playback->seek_to_time(std::chrono::nanoseconds(time));
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

unsigned long long int rs2_playback_get_position(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    return playback->get_position();
}
HANDLE_EXCEPTIONS_AND_RETURN(0, device)

void rs2_playback_device_resume(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    playback->resume();
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

void rs2_playback_device_pause(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    return playback->pause();
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

void rs2_playback_device_set_real_time(const rs2_device* device, int real_time, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    playback->set_real_time(real_time == 0 ? false : true);
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

int rs2_playback_device_is_real_time(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    return playback->is_real_time() ? 1 : 0;
}
HANDLE_EXCEPTIONS_AND_RETURN(0, device)

void rs2_playback_device_set_status_changed_callback(const rs2_device* device, rs2_playback_status_changed_callback* callback, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    VALIDATE_NOT_NULL(callback);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    auto cb = std::shared_ptr<rs2_playback_status_changed_callback>(callback, [](rs2_playback_status_changed_callback* p) { if(p) p->release();});
    playback->playback_status_changed += [cb](rs2_playback_status status){ cb->on_playback_status_changed(status);};
}
HANDLE_EXCEPTIONS_AND_RETURN(, device, callback)


rs2_playback_status rs2_playback_device_get_current_status(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    return playback->get_current_status();
}
HANDLE_EXCEPTIONS_AND_RETURN(RS2_PLAYBACK_STATUS_UNKNOWN, device)

void rs2_playback_device_set_playback_speed(const rs2_device* device, float speed, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    playback->set_frame_rate(speed);
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

void rs2_playback_device_stop(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto playback = VALIDATE_INTERFACE(device->device, librealsense::playback_device);
    return playback->stop();
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

void rs2_record_device_pause(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto record_device = VALIDATE_INTERFACE(device->device, librealsense::record_device);
    record_device->pause_recording();
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

void rs2_record_device_resume(const rs2_device* device, rs2_error** error) try
{
    VALIDATE_NOT_NULL(device);
    auto record_device = VALIDATE_INTERFACE(device->device, librealsense::record_device);
    record_device->resume_recording();
}
HANDLE_EXCEPTIONS_AND_RETURN(, device)

rs2_frame* rs2_allocate_synthetic_video_frame(rs2_source* source, const rs2_stream_profile* new_stream, rs2_frame* original,
    int new_bpp, int new_width, int new_height, int new_stride, rs2_extension frame_type, rs2_error** error) try
{
    VALIDATE_NOT_NULL(source);
    VALIDATE_NOT_NULL(original);
    VALIDATE_NOT_NULL(new_stream);

    auto recovered_profile = std::dynamic_pointer_cast<stream_profile_interface>(new_stream->profile->shared_from_this());

    return (rs2_frame*)source->source->allocate_video_frame(recovered_profile,
        (frame_interface*)original, new_bpp, new_width, new_height, new_stride, frame_type);
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, source, new_stream, original, new_bpp, new_width, new_height, new_stride, frame_type)

void rs2_synthetic_frame_ready(rs2_source* source, rs2_frame* frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);

    librealsense::frame_holder holder((frame_interface*)frame);
    VALIDATE_NOT_NULL(source);

    source->source->frame_ready(std::move(holder));
}
HANDLE_EXCEPTIONS_AND_RETURN(, source, frame)

rs2_pipeline* rs2_create_pipeline(rs2_context* ctx, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(ctx);

    auto pipe = std::make_shared<librealsense::pipeline>(ctx->ctx);

    return new rs2_pipeline{ pipe };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, ctx)

rs2_device* rs2_pipeline_get_device(rs2_context* ctx, rs2_pipeline* pipe, rs2_error ** error)try
{
    VALIDATE_NOT_NULL(pipe);

    return new rs2_device{ ctx->ctx, /*TODO: how does this affect the new device*/ nullptr, pipe->pipe->get_device()};
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, pipe)

void rs2_start_pipeline_with_callback(rs2_pipeline* pipe, rs2_frame_callback* callback, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->start({ callback, [](rs2_frame_callback* p) { p->release(); } });
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe)

void rs2_start_pipeline(rs2_pipeline* pipe, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->start();
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe)

void rs2_open_pipeline(rs2_pipeline* pipe, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->open();
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe)

void rs2_start_pipeline_with_callback_cpp( rs2_pipeline* pipe, rs2_frame_callback* callback, rs2_error** error) try
{
    VALIDATE_NOT_NULL(pipe);
    VALIDATE_NOT_NULL(callback);
    pipe->pipe->start({ callback, [](rs2_frame_callback* p) { p->release(); } });
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe, callback)

void rs2_start_pipeline_with_callback( rs2_pipeline* pipe,  rs2_frame_callback_ptr on_frame, void* user, rs2_error** error) try
{
    VALIDATE_NOT_NULL(pipe);
    VALIDATE_NOT_NULL(on_frame);
    librealsense::frame_callback_ptr callback(
        new librealsense::frame_callback(on_frame, user));
    pipe->pipe->start(move(callback));
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe, on_frame, user)

void rs2_stop_pipeline(rs2_pipeline* pipe, rs2_error ** error)
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->stop();
}

void rs2_enable_pipeline_stream(rs2_pipeline* pipe, rs2_stream stream, int index, int width, int height, rs2_format format, int framerate, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->enable(stream, index,  width,  height,  format, framerate);
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe)

void rs2_enable_pipeline_device(rs2_pipeline* pipe, const char* serial, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->enable(serial);
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe)

void rs2_disable_stream_pipeline(rs2_pipeline* pipe, rs2_stream stream, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->disable_stream(stream);
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe)

void rs2_disable_all_streams_pipeline(rs2_pipeline* pipe, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    pipe->pipe->disable_all();
}
HANDLE_EXCEPTIONS_AND_RETURN(, pipe)


rs2_frame* rs2_pipeline_wait_for_frames(rs2_pipeline* pipe, unsigned int timeout_ms, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    auto f = pipe->pipe->wait_for_frames(timeout_ms);
    auto frame = f.frame;
    f.frame = nullptr;
    return (rs2_frame*)(frame);
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, pipe)

int rs2_pipeline_poll_for_frames(rs2_pipeline * pipe, rs2_frame** output_frame, rs2_error ** error) try
{
    VALIDATE_NOT_NULL(pipe);

    librealsense::frame_holder fh;
    if (pipe->pipe->poll_for_frames(&fh))
    {
        frame_interface* result = nullptr;
        std::swap(result, fh.frame);
        *output_frame = (rs2_frame*)result;
        return true;
    }
    return false;
}
HANDLE_EXCEPTIONS_AND_RETURN(0, pipe, output_frame)

rs2_stream_profile_list* rs2_pipeline_get_active_streams(rs2_pipeline * pipe, rs2_error** error) try
{
    VALIDATE_NOT_NULL(pipe);
    return new rs2_stream_profile_list{ pipe->pipe->get_active_streams() };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, pipe)

const rs2_stream_profile* rs2_pipeline_get_stream_type_selection(const rs2_stream_profile_list* list, rs2_stream stream, int index, rs2_error** error) try
{
    VALIDATE_NOT_NULL(list);
   
    for (auto prof : list->list)
    {
        if(prof->get_stream_type() == stream && prof ->get_stream_index() == index)
            return prof->get_c_wrapper();
    }

    throw librealsense::invalid_value_exception(librealsense::to_string() << "stream " << stream << " is not contained in list!");
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, list, index)

void rs2_delete_pipeline(rs2_pipeline* pipe) try
{
    VALIDATE_NOT_NULL(pipe);

    delete pipe;
}
NOEXCEPT_RETURN(, pipe)


rs2_processing_block* rs2_create_processing_block(rs2_frame_processor_callback* proc, rs2_error** error) try
{
    auto block = std::make_shared<librealsense::processing_block>();
    block->set_processing_callback({ proc, [](rs2_frame_processor_callback* p) { p->release(); } });

    return new rs2_processing_block { block };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, proc)

rs2_processing_block* rs2_create_sync_processing_block(rs2_error** error)
{
    auto block = std::make_shared<librealsense::syncer_proccess_unit>();

    return new rs2_processing_block{ block };
}

void rs2_start_processing(rs2_processing_block* block, rs2_frame_callback* on_frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(block);

    block->block->set_output_callback({ on_frame, [](rs2_frame_callback* p) { p->release(); } });
}
HANDLE_EXCEPTIONS_AND_RETURN(, block, on_frame)

void rs2_process_frame(rs2_processing_block* block, rs2_frame* frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(block);
    VALIDATE_NOT_NULL(frame);

    block->block->invoke(frame_holder((frame_interface*)frame));
}
HANDLE_EXCEPTIONS_AND_RETURN(, block, frame)

void rs2_delete_processing_block(rs2_processing_block* block) try
{
    VALIDATE_NOT_NULL(block);

    delete block;
}
NOEXCEPT_RETURN(, block)

rs2_frame* rs2_extract_frame(rs2_frame* composite, int index, rs2_error** error) try
{
    VALIDATE_NOT_NULL(composite);

    auto cf = VALIDATE_INTERFACE((frame_interface*)composite, librealsense::composite_frame);

    VALIDATE_RANGE(index, 0, (int)cf->get_embedded_frames_count() - 1);
    auto res = cf->get_frame(index);
    res->acquire();
    return (rs2_frame*)res;
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, composite)

rs2_frame* rs2_allocate_composite_frame(rs2_source* source, rs2_frame** frames, int count, rs2_error** error) try
{
    VALIDATE_NOT_NULL(source)
    VALIDATE_NOT_NULL(frames)
    VALIDATE_RANGE(count, 1, 128);

    std::vector<frame_holder> holders(count);
    for (int i = 0; i < count; i++)
    {
        holders[i] = std::move(frame_holder((frame_interface*)frames[i]));
    }
    auto res = source->source->allocate_composite_frame(std::move(holders));

    return (rs2_frame*)res;
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, frames, count)

int rs2_embedded_frames_count(rs2_frame* composite, rs2_error** error) try
{
    VALIDATE_NOT_NULL(composite)

    auto cf = VALIDATE_INTERFACE((frame_interface*)composite, librealsense::composite_frame);

    return static_cast<int>(cf->get_embedded_frames_count());
}
HANDLE_EXCEPTIONS_AND_RETURN(0, composite)

rs2_vertex* rs2_get_frame_vertices(const rs2_frame* frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);
    auto points = VALIDATE_INTERFACE((frame_interface*)frame, librealsense::points);
    return (rs2_vertex*)points->get_vertices();
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, frame)

rs2_pixel* rs2_get_frame_texture_coordinates(const rs2_frame* frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);
    auto points = VALIDATE_INTERFACE((frame_interface*)frame, librealsense::points);
    return (rs2_pixel*)points->get_texture_coordinates();
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, frame)

int rs2_get_frame_points_count(const rs2_frame* frame, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame);
    auto points = VALIDATE_INTERFACE((frame_interface*)frame, librealsense::points);
    return static_cast<int>(points->get_vertex_count());
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame)

rs2_processing_block* rs2_create_pointcloud(rs2_error** error) try
{
    auto block = std::make_shared<librealsense::pointcloud>();

    return new rs2_processing_block { block };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, 0)

rs2_processing_block* rs2_create_align(rs2_stream align_to, rs2_error** error) try
{
    VALIDATE_ENUM(align_to);

    auto block = std::make_shared<librealsense::align>(align_to);
    return new rs2_processing_block{ block };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, align_to)

rs2_processing_block* rs2_create_colorizer(rs2_error** error)
{
    auto block = std::make_shared<librealsense::colorizer>();

    return new rs2_processing_block{ block };
}

float rs2_get_depth_scale(rs2_sensor* sensor, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    auto ds = VALIDATE_INTERFACE(sensor->sensor, librealsense::depth_sensor);
    return ds->get_depth_scale();
}
HANDLE_EXCEPTIONS_AND_RETURN(0.f, sensor)

rs2_device* rs2_create_device_from_sensor(const rs2_sensor* sensor, rs2_error** error) try
{
    VALIDATE_NOT_NULL(sensor);
    return new rs2_device { sensor->parent };
}
HANDLE_EXCEPTIONS_AND_RETURN(nullptr, sensor)

float rs2_depth_frame_get_distance(const rs2_frame* frame_ref, int x, int y, rs2_error** error) try
{
    VALIDATE_NOT_NULL(frame_ref);
    auto df = VALIDATE_INTERFACE(((frame_interface*)frame_ref), librealsense::depth_frame);
    return df->get_distance(x, y);
}
HANDLE_EXCEPTIONS_AND_RETURN(0, frame_ref, x, y)

rs2_time_t rs2_get_time(rs2_error** error) try
{
    return environment::get_instance().get_time_service()->get_time();
}
HANDLE_EXCEPTIONS_AND_RETURN(0,0)
