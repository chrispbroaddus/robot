// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <cmath>
#include "playback_device.h"
#include "core/motion.h"
#include "stream.h"
#include "environment.h"
#include "get_device_index.h"

using namespace device_serializer;

playback_device::playback_device(std::shared_ptr<context> ctx, std::shared_ptr<device_serializer::reader> serializer) :
    m_context(ctx),
    m_is_started(false),
    m_is_paused(false),
    m_sample_rate(1),
    m_real_time(false),
    m_prev_timestamp(0),
    m_read_thread([]() {return std::make_shared<dispatcher>(std::numeric_limits<unsigned int>::max()); })
{
    if (serializer == nullptr)
    {
        throw invalid_value_exception("null serializer");
    }

    m_reader = serializer;
    (*m_read_thread)->start();

    //Read header and build device from recorded device snapshot
    m_device_description = m_reader->query_device_description();
    auto info_snapshot = m_device_description.get_device_extensions_snapshots().find(RS2_EXTENSION_INFO);
    if(info_snapshot == nullptr)
    {
        throw io_exception("Recorded file does not contain device information");
    }
    auto info_api = As<info_interface>(info_snapshot);
    if (info_api == nullptr)
    {
        throw invalid_value_exception("Failed to get info interface from device snapshots");
    }
    register_device_info(info_api);
    //Create playback sensor that simulate the recorded sensors
    m_sensors = create_playback_sensors(m_device_description);

    register_extrinsics(m_device_description);
}

std::map<uint32_t, std::shared_ptr<playback_sensor>> playback_device::create_playback_sensors(const device_snapshot& device_description)
{
    std::map<uint32_t, std::shared_ptr<playback_sensor>> sensors;
    for (auto sensor_snapshot : device_description.get_sensors_snapshots())
    {
        //Each sensor will know its capabilities from the sensor_snapshot
        auto sensor = std::make_shared<playback_sensor>(*this, sensor_snapshot);

        sensor->started += [this](uint32_t id, frame_callback_ptr user_callback) -> void
        {
            (*m_read_thread)->invoke([this, id, user_callback](dispatcher::cancellable_timer c)
            {
                auto it = m_active_sensors.find(id);
                if (it == m_active_sensors.end())
                {
                    m_active_sensors[id] = m_sensors[id];

                    if (m_active_sensors.size() == 1) //On the first sensor that starts, start the reading thread
                    {
                        start();
                    }
                }
            });
        };

        sensor->stopped += [this](uint32_t id, bool invoke_required) -> void
        {
            //stopped could be called by the user (when calling sensor.stop(), main thread==invoke required) or from the reader_thread when
            // reaching eof, or some read error (which means invoke is not required)

            auto action = [this, id]()
            {
                auto it = m_active_sensors.find(id);
                if (it != m_active_sensors.end())
                {
                    m_active_sensors.erase(it);
                    if (m_active_sensors.size() == 0)
                    {
                        stop_internal();
                    }
                }
            };
            if (invoke_required)
            {
                (*m_read_thread)->invoke([action](dispatcher::cancellable_timer c) { action(); });
            }
            else
            {
                action();
            }
        };

        sensor->opened += [this](const std::vector<device_serializer::stream_identifier>& filters) -> void
        {
            (*m_read_thread)->invoke([this, filters](dispatcher::cancellable_timer c)
            {
                m_reader->enable_stream(filters);
            });
        };

        sensor->closed += [this](const std::vector<device_serializer::stream_identifier>& filters) -> void
        {
            (*m_read_thread)->invoke([this, filters](dispatcher::cancellable_timer c)
            {
                m_reader->disable_stream(filters);
            });
        };

        sensors[sensor_snapshot.get_sensor_index()] = sensor;
    }
    return sensors;
}

std::shared_ptr<stream_profile_interface> playback_device::get_stream(const std::map<unsigned, std::shared_ptr<playback_sensor>>& sensors_map, device_serializer::stream_identifier stream_id)
{
    for (auto sensor_pair : sensors_map)
    {
        if(sensor_pair.first == stream_id.sensor_index)
        {
            for (auto stream_profile : sensor_pair.second->get_stream_profiles())
            {
                if(stream_profile->get_stream_type() == stream_id.stream_type && stream_profile->get_stream_index() == stream_id.stream_index)
                {
                    return stream_profile;
                }
            }
        }
    }
    throw invalid_value_exception("File contains extrinsics that do not map to an existing stream");
}

rs2_extrinsics playback_device::calc_extrinsic(const rs2_extrinsics& from, const rs2_extrinsics& to)
{
    //NOTE: Assuming here that recording is writing extrinsics **from** some reference point **to** the stream at hand
    return from_pose(inverse(to_pose(from)) * to_pose(to));
}

playback_device::~playback_device()
{
    (*m_read_thread)->invoke([this](dispatcher::cancellable_timer c)
    {
        for (auto&& sensor : m_active_sensors)
        {
            if (sensor.second != nullptr)
                sensor.second->stop(); //TODO: make sure this works with new dispatcher
        }
    });
    if((*m_read_thread)->flush() == false)
    {
        LOG_ERROR("Error - timeout waiting for flush, possible deadlock detected");
        assert(0); //Detect this immediately in debug
    }
    (*m_read_thread)->stop();
}

std::shared_ptr<context> playback_device::get_context() const
{
    return m_context;
}

sensor_interface& playback_device::get_sensor(size_t i)
{
    return *m_sensors.at(static_cast<uint32_t>(i));
}

size_t playback_device::get_sensors_count() const
{
    return m_sensors.size();
}

const sensor_interface& playback_device::get_sensor(size_t i) const
{
    auto sensor = m_sensors.at(static_cast<uint32_t>(i));
    return *std::dynamic_pointer_cast<sensor_interface>(sensor);
}

void playback_device::hardware_reset()
{
    //Nothing to see here folks
}

bool playback_device::extend_to(rs2_extension extension_type, void** ext)
{
    std::shared_ptr<extension_snapshot> e = m_device_description.get_device_extensions_snapshots().find(extension_type);
    if (e == nullptr)
    {
        return false;
    }
    switch (extension_type)
    {
    case RS2_EXTENSION_UNKNOWN: return false;
    case RS2_EXTENSION_DEBUG : return try_extend<debug_interface>(e, ext);
    case RS2_EXTENSION_INFO : return try_extend<info_interface>(e, ext);
    case RS2_EXTENSION_MOTION : return try_extend<motion_sensor_interface>(e, ext);;
    case RS2_EXTENSION_OPTIONS : return try_extend<options_interface>(e, ext);;
    case RS2_EXTENSION_VIDEO : return try_extend<video_sensor_interface>(e, ext);;
    case RS2_EXTENSION_ROI : return try_extend<roi_sensor_interface>(e, ext);;
    case RS2_EXTENSION_VIDEO_FRAME : return try_extend<video_frame>(e, ext);
    //TODO: add: case RS2_EXTENSION_MOTION_FRAME : return try_extend<motion_frame>(e, ext);
    case RS2_EXTENSION_COUNT :
        //[[fallthrough]];
    default:
        LOG_WARNING("Unsupported extension type: " << extension_type);
        return false;
    }
}

std::shared_ptr<matcher> playback_device::create_matcher(const frame_holder& frame) const
{
    return nullptr; //TOOD: WTD?
}

void playback_device::set_frame_rate(double rate)
{
    if(rate < 0)
    {
        throw invalid_value_exception(to_string() << "Failed to set frame rate to " << std::to_string(rate) << ", value is less than 0");
    }
    (*m_read_thread)->invoke([this, rate](dispatcher::cancellable_timer t)
    {
        LOG_INFO("Changing playback frame rate to: " << rate);
        m_sample_rate = rate;
        update_time_base(m_prev_timestamp);
    });
}

void playback_device::seek_to_time(std::chrono::nanoseconds time)
{
    (*m_read_thread)->invoke([this, time](dispatcher::cancellable_timer t)
    {
        LOG_INFO("Seek to time: " << time.count());
        m_reader->seek_to_time(time);
        m_prev_timestamp = time; //Updating prev timestamp to make get_position return true indication even when playbakc is paused
        catch_up();
    });
    (*m_read_thread)->flush();
}

rs2_playback_status playback_device::get_current_status() const
{
    return m_is_started ?
           m_is_paused ? RS2_PLAYBACK_STATUS_PAUSED : RS2_PLAYBACK_STATUS_PLAYING
                        : RS2_PLAYBACK_STATUS_STOPPED;
}

uint64_t playback_device::get_duration() const
{
    return m_reader->query_duration().count();
}

void playback_device::pause()
{
    LOG_DEBUG("Playback Pause called");

    /*
        Playing ---->  pause()   set m_is_paused  to True  ----> Paused
        Paused  ---->  pause()   set m_is_paused  to True  ----> Do nothing
        Stopped ---->  pause()   set m_is_paused  to True  ----> Do nothing
    */
    (*m_read_thread)->invoke([this](dispatcher::cancellable_timer t)
    {
        LOG_DEBUG("Playback pause invoked");

       if (m_is_paused)
           return;

       m_is_paused = true;

       if(m_is_started)
       {
           //Wait for any remaining sensor callbacks to return
           for (auto sensor : m_sensors)
           {
               sensor.second->flush_pending_frames();
           }
       }
       LOG_DEBUG("Notifying RS2_PLAYBACK_STATUS_PAUSED");
       playback_status_changed(RS2_PLAYBACK_STATUS_PAUSED);
    });
    (*m_read_thread)->flush();
    LOG_INFO("Playback Paused");
}

void playback_device::resume()
{
    LOG_DEBUG("Playback resume called");
    (*m_read_thread)->invoke([this](dispatcher::cancellable_timer t)
    {
        LOG_DEBUG("Playback resume invoked");
        if (m_is_paused == false)
           return;

        m_is_paused = false;
        catch_up();

        try_looping();
    });
    (*m_read_thread)->flush();
    LOG_INFO("Playback Resumed");
}

void playback_device::set_real_time(bool real_time)
{
    LOG_INFO("Set real time to " << real_time ? "True" : "Fales");
    m_real_time = real_time;
}

bool playback_device::is_real_time() const
{
    return m_real_time;
}

platform::backend_device_group playback_device::get_device_data() const
{
    return {};
}

std::pair<uint32_t, rs2_extrinsics> playback_device::get_extrinsics(const stream_interface& stream) const
{
    return m_extrinsics_map.at(stream.get_unique_id());
}

void playback_device::update_time_base(device_serializer::nanoseconds base_timestamp)
{
    m_base_sys_time = std::chrono::high_resolution_clock::now();
    m_base_timestamp = base_timestamp;
    LOG_DEBUG("Updating Time Base... m_base_sys_time " << m_base_sys_time.time_since_epoch().count() << " m_base_timestamp " << m_base_timestamp.count());
}

device_serializer::nanoseconds playback_device::calc_sleep_time(device_serializer::nanoseconds timestamp) const
{
    //The time to sleep returned here equals to the difference between the file recording time
    // and the playback time.
    auto now = std::chrono::high_resolution_clock::now();
    auto play_time = now - m_base_sys_time;
    if(timestamp < m_base_timestamp)
    {
        assert(0);
    }
    auto time_diff = timestamp - m_base_timestamp;
    auto recorded_time = std::chrono::duration_cast<device_serializer::nanoseconds>(time_diff / m_sample_rate.load());
    
    LOG_DEBUG("Time Now  : " << now.time_since_epoch().count() << " ,    Time When Started: " << m_base_sys_time.time_since_epoch().count() << " , Diff: " << play_time.count() << " == " << (play_time.count()/1000)/1000 << "ms");
    LOG_DEBUG("Original Recording Delta: " << time_diff.count() << " == " << (time_diff.count() / 1000) / 1000 << "ms");
    LOG_DEBUG("Frame Time: " << timestamp.count() << "  , First Frame: " << m_base_timestamp.count() << " ,  Diff: " << recorded_time.count() << " == " << (recorded_time.count() / 1000) / 1000 << "ms");

    if(recorded_time < play_time)
    {
        LOG_DEBUG("Recorded Time < Playing Time  (not sleeping)");
        return device_serializer::nanoseconds(0);
    }
    auto sleep_time = (recorded_time - play_time);
    LOG_DEBUG("Sleep Time: " << sleep_time.count() << " == " << (sleep_time.count() / 1000) / 1000 << " ms");
    return sleep_time;
}

void playback_device::start()
{
    //Start reading from the file
    //Start is called only from Stopped state
    /*
    Playing ---->  start()   set m_is_started to True  ----> Do nothing
    Paused  ---->  start()   set m_is_started to True  ----> Do nothing
    Stopped ---->  start()   set m_is_started to True  ----> Paused/Playing (depends on m_is_paused)
    */
    LOG_DEBUG("playback start called");

    if (m_is_started)
        return ; //nothing to do

    m_is_started = true;
    catch_up();
    try_looping();
    LOG_INFO("Playback started");

}
void playback_device::stop()
{
    LOG_DEBUG("playback stop called");
    (*m_read_thread)->invoke([this](dispatcher::cancellable_timer t)
    {
        LOG_DEBUG("playback stop invoked");
        stop_internal();
    });
    (*m_read_thread)->flush();
    LOG_INFO("Playback stoped");

}

void playback_device::stop_internal()
{
    //stop_internal() is called from within the reading thread
    if (m_is_started == false)
        return; //nothing to do


    m_is_started = false;
    m_is_paused = false;
    for (auto sensor : m_sensors)
    {
        //sensor.second->flush_pending_frames();
    }
    m_reader->reset();
    m_prev_timestamp = std::chrono::nanoseconds(0);
    catch_up();
    playback_status_changed(RS2_PLAYBACK_STATUS_STOPPED);
}

template <typename T>
void playback_device::do_loop(T action)
{
    (*m_read_thread)->invoke([this, action](dispatcher::cancellable_timer c)
    {
        bool action_succeeded = false;
        try
        {
            action_succeeded = action();
        }
        catch(const std::exception& e)
        {
            LOG_ERROR("Failed to read next frame from file: " << e.what());
            //TODO: notify user that playback unexpectedly ended
            action_succeeded = false; //will make the scope_guard stop the sensors, must return.
        }

        //On failure, exit thread
        if(action_succeeded == false)
        {
            //Go over the sensors and stop them
            size_t active_sensors_count = m_active_sensors.size();
            for (size_t i = 0; i<active_sensors_count; i++)
            {
                if (m_active_sensors.size() == 0)
                    break;

                //NOTE: calling stop will remove the sensor from m_active_sensors
                m_active_sensors.begin()->second->stop(false);
            }
            //After all sensors were stopped, stop_internal() is called and flags m_is_started as false
            assert(m_is_started == false);
        }

        //Continue looping?
        if (m_is_started == true && m_is_paused == false)
        {
            do_loop(action);
        }
    });
}

void playback_device::try_looping()
{
    //try_looping is called from start() or resume()
    if (m_is_started && m_is_paused == false)
    {
        //Notify subscribers that playback status changed
        if (m_is_paused)
        {
            playback_status_changed(RS2_PLAYBACK_STATUS_PAUSED);
        }
        else
        {
            playback_status_changed(RS2_PLAYBACK_STATUS_PLAYING);
        }
    }
    auto read_action = [this]() -> bool
    {
        LOG_DEBUG("Read action invoked");

        //Read next data from the serializer, on success: 'obj' will be a valid object that came from
        // sensor number 'sensor_index' with a timestamp equal to 'timestamp'
        device_serializer::stream_identifier stream_id;
        device_serializer::nanoseconds timestamp = device_serializer::nanoseconds::max();
        frame_holder frame;
        auto retval = m_reader->read_frame(timestamp, stream_id, frame);
        if (retval == device_serializer::status_file_eof)
        {
            LOG_INFO("End of file reached");
            return false;
        }
        if(retval != device_serializer::status_no_error || timestamp == std::chrono::nanoseconds::max() || frame == nullptr)
        {
            LOG_ERROR("Failed to read next frame. retval: " << retval << ", timestamp = " << timestamp.count());
            throw librealsense::io_exception("Failed to read frame");
        }

        m_prev_timestamp = timestamp;
        //Objects with timestamp of 0 are non streams.
        if (m_base_timestamp.count() == 0)
        {
            //As long as m_base_timestamp is 0, update it to object's timestamp.
            //Once a streaming object arrive, the base will change from 0
            update_time_base(timestamp);
        }

        //Calculate the duration for the reader to sleep (i.e wait for next frame)
        auto sleep_time = calc_sleep_time(timestamp);
        if (sleep_time.count() > 0)
        {
            if (m_sample_rate > 0)
            {
                LOG_DEBUG("Sleeping for: " << (sleep_time.count() / 1000) / 1000);
                std::this_thread::sleep_for(sleep_time);
            }
        }

        if (stream_id.device_index != get_device_index() || stream_id.sensor_index >= m_sensors.size())
        {
            LOG_ERROR("Unexpected sensor index while playing file (Read index = " << stream_id.sensor_index << ")");
            throw invalid_value_exception(to_string() << "Unexpected sensor index while playing file (Read index = " << stream_id.sensor_index << ")");
        }
        LOG_DEBUG("Dispatching frame " << stream_id.device_index << "/" << stream_id.sensor_index << "/" << stream_id.stream_type << "/" << stream_id.stream_index);
        //Dispatch frame to the relevant sensor
        m_sensors[stream_id.sensor_index]->handle_frame(std::move(frame), m_real_time);
        return true;
    };
    do_loop(read_action);
}

const std::string& playback_device::get_file_name() const
{
    return m_reader->get_file_name();
}

uint64_t playback_device::get_position() const
{
    return m_prev_timestamp.count();
}
void playback_device::catch_up()
{
    m_base_timestamp = std::chrono::microseconds(0);
    LOG_DEBUG("Catching up");
}

void playback_device::register_device_info(const std::shared_ptr<info_interface>& info_api)
{
    for (int i = 0; i < RS2_CAMERA_INFO_COUNT; ++i)
    {
        rs2_camera_info info = static_cast<rs2_camera_info>(i);
        if (info_api->supports_info(info))
        {
            register_info(info, info_api->get_info(info));
        }
    }
}

void playback_device::register_extrinsics(const device_serializer::device_snapshot& device_description)
{
    //Register extrinsics
    for (auto e1 : device_description.get_extrinsics_map())
    {
        for (auto e2 : device_description.get_extrinsics_map())
        {
            if (e1.second.first != e2.second.first)
            {
                //Not under the same extrinsics group
                continue;
            }

            auto p1 = get_stream(m_sensors, e1.first);
            auto p2 = get_stream(m_sensors, e2.first);
            rs2_extrinsics x = calc_extrinsic(e1.second.second, e2.second.second);
            auto extrinsic_fetcher = std::make_shared<lazy<rs2_extrinsics>>([x]()
            {
                return x;
            });
            m_extrinsics_map[p1->get_unique_id()] = e1.second;
            m_extrinsics_map[p2->get_unique_id()] = e2.second;
            environment::get_instance().get_extrinsics_graph().register_extrinsics(*p1, *p2, extrinsic_fetcher);
            m_extrinsics_fetchers.push_back(extrinsic_fetcher);  //Caching the lazy<rs2_extrinsics> since context holds weak_ptr
        }
    }
}
