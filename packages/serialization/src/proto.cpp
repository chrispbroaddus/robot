#include <fcntl.h>
#include <string>
#include <sys/types.h>

#include "glog/logging.h"

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"

#include "packages/serialization/include/proto.h"

namespace serialization {

bool loadProtoText(const std::string& path, google::protobuf::Message* pb) {
    // Read in a file and print its contents to stdout.
    int fd = open(path.c_str(), O_RDONLY);
    if (!fd) {
        LOG(ERROR) << "unable to open " << path;
        return false;
    }

    google::protobuf::io::FileInputStream input(fd);
    return google::protobuf::TextFormat::Parse(&input, pb);
}

} // namespace serialization
