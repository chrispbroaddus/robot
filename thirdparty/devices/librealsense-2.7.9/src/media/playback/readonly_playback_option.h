#pragma once

#include "librealsense2/rs.h"
#include "option.h"

namespace librealsense {
    class read_only_playback_option : public readonly_option {
        float _value;
        rs2_option _type;
        std::string _description;
    public:
        read_only_playback_option(rs2_option type, float value) : _value(value), _type(type) {
            _description = to_string() << "Read only option of " << rs2_option_to_string(type);
        }

        float query() const override {
            return _value;
        }

        option_range get_range() const override {
            return {_value, _value, 0, _value};
        }

        bool is_enabled() const override {
            return true;
        }

        const char *get_description() const override {
            return _description.c_str();
        }
    };
}