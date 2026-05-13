#pragma once

#include <any>
#include <map>
#include <string>

#include <google/protobuf/struct.pb.h>

namespace evrp::sdk {

void fromProto(std::map<std::string, std::any>& out,
               const google::protobuf::Struct& s);

void toProto(const std::map<std::string, std::any>& snap,
             google::protobuf::Struct* out);

}  // namespace evrp::sdk
