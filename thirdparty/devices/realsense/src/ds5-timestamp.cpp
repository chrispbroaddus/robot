// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include "ds5-private.h"
#include "ds5-timestamp.h"

#include <mutex>
#include <chrono>
#include <vector>
#include <iterator>

#include "device.h"
#include "context.h"
#include "image.h"

namespace rsimpl2
{
    ds5_timestamp_reader_from_metadata::ds5_timestamp_reader_from_metadata(std::unique_ptr<frame_timestamp_reader> backup_timestamp_reader)
        :_backup_timestamp_reader(std::move(backup_timestamp_reader)), _has_metadata(pins)
    {
        reset();
    }

    bool ds5_timestamp_reader_from_metadata::has_metadata(const request_mapping& mode, const void * metadata, size_t metadata_size)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if(metadata == nullptr || metadata_size == 0)
        {
            return false;
        }

        for(uint32_t i=0; i<metadata_size; i++)
        {
            if(((byte*)metadata)[i] != 0)
            {
                return true;
            }
        }
        return false;
    }

    rs2_time_t ds5_timestamp_reader_from_metadata::get_frame_timestamp(const request_mapping& mode, const uvc::frame_object& fo)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        auto pin_index = 0;
        if (mode.pf->fourcc == 0x5a313620) // Z16
            pin_index = 1;

        if(!_has_metadata[pin_index])
        {
           _has_metadata[pin_index] = has_metadata(mode, fo.metadata, fo.metadata_size);
        }

        if(_has_metadata[pin_index])
        {
            auto md = (rsimpl2::ds::metadata*)(fo.metadata);
            return (double)(md->header.timestamp)*TIMESTAMP_TO_MILLISECONS;
        }
        else
        {
            if (!started)
            {
                LOG_WARNING("UVC timestamp not found! please apply UVC metadata patch.");
                started = true;
            }
            return _backup_timestamp_reader->get_frame_timestamp(mode, fo);
        }
    }

    unsigned long long ds5_timestamp_reader_from_metadata::get_frame_counter(const request_mapping & mode, const uvc::frame_object& fo) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        auto pin_index = 0;
        if (mode.pf->fourcc == 0x5a313620) // Z16
            pin_index = 1;

        if(_has_metadata[pin_index])
        {
            auto md = (rsimpl2::ds::metadata*)(fo.metadata);
            return md->md_capture_timing.frameCounter;
        }
        else
        {
            return _backup_timestamp_reader->get_frame_counter(mode, fo);
        }
    }

    void ds5_timestamp_reader_from_metadata::reset()
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        started = false;
        for (auto i = 0; i < pins; ++i)
        {
            _has_metadata[i] = false;
        }
    }

    rs2_timestamp_domain ds5_timestamp_reader_from_metadata::get_frame_timestamp_domain(const request_mapping & mode, const uvc::frame_object& fo) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        auto pin_index = 0;
        if (mode.pf->fourcc == 0x5a313620) // Z16
            pin_index = 1;

        return _has_metadata[pin_index] ? RS2_TIMESTAMP_DOMAIN_HARDWARE_CLOCK :
                                          _backup_timestamp_reader->get_frame_timestamp_domain(mode,fo);
    }

    ds5_timestamp_reader::ds5_timestamp_reader(std::shared_ptr<uvc::time_service> ts)
        : counter(pins), _ts(ts)
    {
        reset();
    }

    void ds5_timestamp_reader::reset()
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        for (auto i = 0; i < pins; ++i)
        {
            counter[i] = 0;
        }
    }

    rs2_time_t ds5_timestamp_reader::get_frame_timestamp(const request_mapping& mode, const uvc::frame_object& fo)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        return _ts->get_time();
    }

    unsigned long long ds5_timestamp_reader::get_frame_counter(const request_mapping & mode, const uvc::frame_object& fo) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        auto pin_index = 0;
        if (mode.pf->fourcc == 0x5a313620) // Z16
            pin_index = 1;

        return ++counter[pin_index];
    }

    rs2_timestamp_domain ds5_timestamp_reader::get_frame_timestamp_domain(const request_mapping & mode, const uvc::frame_object& fo) const
    {
        return RS2_TIMESTAMP_DOMAIN_SYSTEM_TIME;
    }

    ds5_iio_hid_timestamp_reader::ds5_iio_hid_timestamp_reader()
    {
        counter.resize(sensors);
        reset();
    }

    void ds5_iio_hid_timestamp_reader::reset()
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        started = false;
        for (auto i = 0; i < sensors; ++i)
        {
            counter[i] = 0;
        }
    }

    rs2_time_t ds5_iio_hid_timestamp_reader::get_frame_timestamp(const request_mapping& mode, const uvc::frame_object& fo)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if(has_metadata(mode, fo.metadata, fo.metadata_size))
        {
            auto timestamp = *((uint64_t*)((const uint8_t*)fo.metadata));
            return static_cast<rs2_time_t>(timestamp) * TIMESTAMP_TO_MILLISECONS;
        }

        if (!started)
        {
            LOG_WARNING("HID timestamp not found! please apply HID patch.");
            started = true;
        }

        return std::chrono::duration<rs2_time_t, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    bool ds5_iio_hid_timestamp_reader::has_metadata(const request_mapping& mode, const void * metadata, size_t metadata_size) const
    {
        if(metadata != nullptr && metadata_size > 0)
        {
            return true;
        }
        return false;
    }

    unsigned long long ds5_iio_hid_timestamp_reader::get_frame_counter(const request_mapping & mode, const uvc::frame_object& fo) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        int index = 0;
        if (mode.pf->fourcc == 'GYRO')
            index = 1;

        return ++counter[index];
    }

    rs2_timestamp_domain ds5_iio_hid_timestamp_reader::get_frame_timestamp_domain(const request_mapping & mode, const uvc::frame_object& fo) const
    {
        if(has_metadata(mode ,fo.metadata, fo.metadata_size))
        {
            return RS2_TIMESTAMP_DOMAIN_HARDWARE_CLOCK;
        }
        return RS2_TIMESTAMP_DOMAIN_SYSTEM_TIME;
    }

    ds5_custom_hid_timestamp_reader::ds5_custom_hid_timestamp_reader()
    {
        counter.resize(sensors);
        reset();
    }

    void ds5_custom_hid_timestamp_reader::reset()
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        for (auto i = 0; i < sensors; ++i)
        {
            counter[i] = 0;
        }
    }

    rs2_time_t ds5_custom_hid_timestamp_reader::get_frame_timestamp(const request_mapping& /*mode*/, const uvc::frame_object& fo)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        static const uint8_t timestamp_offset = 17;

        auto timestamp = *((uint64_t*)((const uint8_t*)fo.pixels + timestamp_offset));
        return static_cast<rs2_time_t>(timestamp) * TIMESTAMP_TO_MILLISECONS;
    }

    bool ds5_custom_hid_timestamp_reader::has_metadata(const request_mapping& /*mode*/, const void * /*metadata*/, size_t /*metadata_size*/) const
    {
        return true;
    }

    unsigned long long ds5_custom_hid_timestamp_reader::get_frame_counter(const request_mapping & mode, const uvc::frame_object& fo) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        return ++counter.front();
    }

    rs2_timestamp_domain ds5_custom_hid_timestamp_reader::get_frame_timestamp_domain(const request_mapping & /*mode*/, const uvc::frame_object& /*fo*/) const
    {
        return RS2_TIMESTAMP_DOMAIN_HARDWARE_CLOCK;
    }
}
