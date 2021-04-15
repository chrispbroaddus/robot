#pragma once

#include <string>
#include <sys/types.h>

#include "google/protobuf/message.h"

namespace serialization {

// loadProtoText reads a protocol buffer from a file in text format
bool loadProtoText(const std::string& path, google::protobuf::Message* pb);

} // namespace serialization
