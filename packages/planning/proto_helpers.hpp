template <typename T> bool loadOptions(const std::string& filename, T& options, SerializationType type) {
    std::string serialized_proto = loadProtoText(filename);
    CHECK(!serialized_proto.empty());
    switch (type) {
    case SerializationType::TEXT:
        return google::protobuf::TextFormat::MergeFromString(serialized_proto, &options);
    case SerializationType::JSON:
        return google::protobuf::util::JsonStringToMessage(serialized_proto, &options).ok();
    default:
        LOG(FATAL) << "Unknown type!";
        return false;
    }
}
