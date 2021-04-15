#pragma once

#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"

namespace planning {

/*
 *Protobuf utilities
 */
std::string loadProtoText(const std::string& file);

enum class SerializationType { TEXT, JSON };

template <typename T> bool loadOptions(const std::string& filename, T& proto, SerializationType type = SerializationType::TEXT);

#include "proto_helpers.hpp"

} // planning
