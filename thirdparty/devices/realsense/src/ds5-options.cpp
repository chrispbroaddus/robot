// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include "ds5-options.h"

namespace rsimpl2
{
    const char* emitter_option::get_value_description(float val) const
    {
        switch (static_cast<int>(val))
        {
            case 0:
            {
                return "Off";
            }
            case 1:
            {
                return "On";
            }
            case 2:
            {
                return "Auto";
            }
            default:
                throw invalid_value_exception("value not found");
        }
    }

    emitter_option::emitter_option(uvc_endpoint& ep)
        : uvc_xu_option(ep, ds::depth_xu, ds::DS5_DEPTH_EMITTER_ENABLED,
                        "Power of the DS5 projector, 0 meaning projector off, 1 meaning projector on, 2 meaning projector in auto mode")
    {}

    float asic_and_projector_temperature_options::query() const
    {
        if (!is_enabled())
            throw wrong_api_call_sequence_exception("query option is allow only in streaming!");

        #pragma pack(push, 1)
        struct temperature
        {
            uint8_t is_projector_valid;
            uint8_t is_asic_valid;
            int8_t projector_temperature;
            int8_t asic_temperature;
        };
        #pragma pack(pop)

        auto temperature_data = static_cast<temperature>(_ep.invoke_powered(
            [this](uvc::uvc_device& dev)
            {
                temperature temp{};
                if (!dev.get_xu(ds::depth_xu,
                                ds::DS5_ASIC_AND_PROJECTOR_TEMPERATURES,
                                reinterpret_cast<uint8_t*>(&temp),
                                sizeof(temperature)))
                 {
                        throw invalid_value_exception(to_string() << "get_xu(ctrl=DS5_ASIC_AND_PROJECTOR_TEMPERATURES) failed!" << " Last Error: " << strerror(errno));
                 }

                return temp;
            }));

        int8_t temperature::* feild;
        uint8_t temperature::* is_valid_feild;

        switch (_option)
        {
        case RS2_OPTION_ASIC_TEMPERATURE:
            feild = &temperature::asic_temperature;
            is_valid_feild = &temperature::is_asic_valid;
            break;
        case RS2_OPTION_PROJECTOR_TEMPERATURE:
            feild = &temperature::projector_temperature;
            is_valid_feild = &temperature::is_projector_valid;
            break;
        default:
            throw invalid_value_exception(to_string() << rs2_option_to_string(_option) << " is not temperature option!");
        }

        if (!static_cast<bool>(temperature_data.*is_valid_feild))
            throw invalid_value_exception(to_string() << rs2_option_to_string(_option) << " value is not valid!");

        return temperature_data.*feild;
    }

    option_range asic_and_projector_temperature_options::get_range() const
    {
        return option_range { -40, 125, 0, 0 };
    }

    bool asic_and_projector_temperature_options::is_enabled() const
    {
        return _ep.is_streaming();
    }

    const char* asic_and_projector_temperature_options::get_description() const
    {
        switch (_option)
        {
        case RS2_OPTION_ASIC_TEMPERATURE:
            return "Current Asic Temperature";
        case RS2_OPTION_PROJECTOR_TEMPERATURE:
            return "Current Projector Temperature";
        default:
            throw invalid_value_exception(to_string() << rs2_option_to_string(_option) << " is not temperature option!");
        }
    }

    asic_and_projector_temperature_options::asic_and_projector_temperature_options(uvc_endpoint& ep, rs2_option opt)
        : _option(opt), _ep(ep)
        {}

    float motion_module_temperature_option::query() const
    {
        if (!is_enabled())
            throw wrong_api_call_sequence_exception("query option is allow only in streaming!");

        static const auto report_field = uvc::custom_sensor_report_field::value;
        auto data = _ep.get_custom_report_data(custom_sensor_name, report_name, report_field);
        if (data.empty())
            throw invalid_value_exception("query() motion_module_temperature_option failed! Empty buffer arrived.");

        auto data_str = std::string(reinterpret_cast<char const*>(data.data()));
        return std::stof(data_str);
    }

    option_range motion_module_temperature_option::get_range() const
    {
        if (!is_enabled())
            throw wrong_api_call_sequence_exception("get option range is allow only in streaming!");

        static const auto min_report_field = uvc::custom_sensor_report_field::minimum;
        static const auto max_report_field = uvc::custom_sensor_report_field::maximum;
        auto min_data = _ep.get_custom_report_data(custom_sensor_name, report_name, min_report_field);
        if (min_data.empty())
            throw invalid_value_exception("get_range() motion_module_temperature_option failed! Empty buffer arrived.");

        auto max_data = _ep.get_custom_report_data(custom_sensor_name, report_name, max_report_field);
        if (max_data.empty())
            throw invalid_value_exception("get_range() motion_module_temperature_option failed! Empty buffer arrived.");

        auto min_str = std::string(reinterpret_cast<char const*>(min_data.data()));
        auto max_str = std::string(reinterpret_cast<char const*>(max_data.data()));

        return option_range{std::stof(min_str),
                            std::stof(max_str),
                            0, 0};
    }

    bool motion_module_temperature_option::is_enabled() const
    {
        return _ep.is_streaming();
    }

    const char* motion_module_temperature_option::get_description() const
    {
        return "Current Motion-Module Temperature";
    }

    motion_module_temperature_option::motion_module_temperature_option(hid_endpoint& ep)
        : _ep(ep)
    {}

    void enable_motion_correction::set(float value)
    {
        _is_enabled = value > 0;
    }

    float enable_motion_correction::query() const
    {
        auto is_enabled = _is_enabled.load();
        return is_enabled ? 1.f : 0.f;
    }

    enable_motion_correction::enable_motion_correction(endpoint* mm_ep,
                                                       ds::imu_intrinsics accel,
                                                       ds::imu_intrinsics gyro)
        : _is_enabled(true), _accel(accel), _gyro(gyro)
    {
        mm_ep->register_on_before_frame_callback(
                    [this](rs2_stream stream, rs2_frame& f, callback_invocation_holder callback)
        {
            auto fr = f.get();
            if (_is_enabled.load() && fr->get_format() == RS2_FORMAT_MOTION_XYZ32F)
            {
                auto xyz = (float*)(fr->get_frame_data());

                if (stream == RS2_STREAM_ACCEL)
                {
                    for (int i = 0; i < 3; i++)
                        xyz[i] = xyz[i] * _accel.scale[i] + _accel.bias[i];
                }

                if (stream == RS2_STREAM_GYRO)
                {
                    for (int i = 0; i < 3; i++)
                        xyz[i] = xyz[i] * _gyro.scale[i] + _gyro.bias[i];
                }
            }
        });
    }

    void enable_auto_exposure_option::set(float value)
    {
        if (value <0 ) throw invalid_value_exception("Invalid Auto-Exposure mode request " + std::to_string(value));

        auto auto_exposure_prev_state = _auto_exposure_state->get_enable_auto_exposure();
        _auto_exposure_state->set_enable_auto_exposure(0.f < std::fabs(value));

        if (_auto_exposure_state->get_enable_auto_exposure()) // auto_exposure current value
        {
            if (!auto_exposure_prev_state) // auto_exposure previous value
            {
                _to_add_frames = true; // auto_exposure moved from disable to enable
            }
        }
        else
        {
            if (auto_exposure_prev_state)
            {
                _to_add_frames = false; // auto_exposure moved from enable to disable
            }
        }
    }

    float enable_auto_exposure_option::query() const
    {
        return _auto_exposure_state->get_enable_auto_exposure();
    }

    enable_auto_exposure_option::enable_auto_exposure_option(uvc_endpoint* fisheye_ep,
                                std::shared_ptr<auto_exposure_mechanism> auto_exposure,
                                std::shared_ptr<auto_exposure_state> auto_exposure_state)
        : _auto_exposure_state(auto_exposure_state),
          _to_add_frames((_auto_exposure_state->get_enable_auto_exposure())),
          _auto_exposure(auto_exposure)
    {
        fisheye_ep->register_on_before_frame_callback(
                    [this](rs2_stream stream, rs2_frame& f, callback_invocation_holder callback)
        {
            if (!_to_add_frames || stream != RS2_STREAM_FISHEYE)
                return;

            _auto_exposure->add_frame(f.get()->get_owner()->clone_frame(&f), std::move(callback));
        });
    }

    auto_exposure_mode_option::auto_exposure_mode_option(
                              std::shared_ptr<auto_exposure_mechanism> auto_exposure,
                              std::shared_ptr<auto_exposure_state> auto_exposure_state)
        : _auto_exposure_state(auto_exposure_state),
          _auto_exposure(auto_exposure)
    {}

    void auto_exposure_mode_option::set(float value)
    {
        _auto_exposure_state->set_auto_exposure_mode(static_cast<auto_exposure_modes>((int)value));
        _auto_exposure->update_auto_exposure_state(*_auto_exposure_state);
    }

    float auto_exposure_mode_option::query() const
    {
        return static_cast<float>(_auto_exposure_state->get_auto_exposure_mode());
    }

    const char* auto_exposure_mode_option::get_value_description(float val) const
    {
        switch (static_cast<int>(val))
        {
            case 0:
            {
                return "Static";
            }
            case 1:
            {
                return "Anti-Flicker";
            }
            case 2:
            {
                return "Hybrid";
            }
            default:
                throw invalid_value_exception("value not found");
        }
    }

    auto_exposure_antiflicker_rate_option::auto_exposure_antiflicker_rate_option(
                                          std::shared_ptr<auto_exposure_mechanism> auto_exposure,
                                          std::shared_ptr<auto_exposure_state> auto_exposure_state)
        : _auto_exposure_state(auto_exposure_state),
          _auto_exposure(auto_exposure)
    {}

    void auto_exposure_antiflicker_rate_option::set(float value)
    {
        _auto_exposure_state->set_auto_exposure_antiflicker_rate(static_cast<uint32_t>(value));
        _auto_exposure->update_auto_exposure_state(*_auto_exposure_state);
    }

    float auto_exposure_antiflicker_rate_option::query() const
    {
        return static_cast<float>(_auto_exposure_state->get_auto_exposure_antiflicker_rate());
    }

    const char* auto_exposure_antiflicker_rate_option::get_value_description(float val) const
    {
        switch (static_cast<int>(val))
        {
            case 50:
            {
                return "50Hz";
            }
            case 60:
            {
                return "60Hz";
            }
            default:
                throw invalid_value_exception("antiflicker_rate: get_value_description(...) failed. value not found!");
        }
    }

    ds::depth_table_control depth_scale_option::get_depth_table(ds::advnaced_query_mode mode) const
    {
        command cmd(ds::GET_ADV);
        cmd.param1 = ds::etDepthTableControl;
        cmd.param2 = mode;
        auto res = _hwm.send(cmd);

        if (res.size() < sizeof(ds::depth_table_control))
            throw std::runtime_error("Not enough bytes returned from the firmware!");

        auto table = (const ds::depth_table_control*)res.data();
        return *table;
    }

    depth_scale_option::depth_scale_option(hw_monitor& hwm)
        : _hwm(hwm)
    {
        _range = [this]()
        {
            auto min = get_depth_table(ds::GET_MIN);
            auto max = get_depth_table(ds::GET_MAX);
            return option_range{ (float)(0.000001 * min.depth_units),
                                 (float)(0.000001 * max.depth_units),
                                 0.000001f, 0.001f };
        };
    }

    void depth_scale_option::set(float value)
    {
        command cmd(ds::SET_ADV);
        cmd.param1 = ds::etDepthTableControl;

        auto depth_table = get_depth_table(ds::GET_VAL);
        depth_table.depth_units = 1000000 * value;
        auto ptr = (uint8_t*)(&depth_table);
        cmd.data = std::vector<uint8_t>(ptr, ptr + sizeof(ds::depth_table_control));

        _hwm.send(cmd);
    }

    float depth_scale_option::query() const
    {
        auto table = get_depth_table(ds::GET_VAL);
        return (float)(0.000001 * table.depth_units);
    }

    option_range depth_scale_option::get_range() const
    {
        return *_range;
    }
}
