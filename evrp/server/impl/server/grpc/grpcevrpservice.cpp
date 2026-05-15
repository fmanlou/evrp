#include "evrp/server/impl/server/grpc/grpcevrpservice.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <google/protobuf/struct.pb.h>

#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/tofromproto.h"
#include "evrp/server/api/evrp.h"

namespace {

using evrp::sdk::DeviceKind;

void mergeIntoMemorySetting(MemorySetting& settings,
                            const std::map<std::string, std::any>& fields) {
  for (const auto& entry : fields) {
    settings.insert(entry.first, entry.second);
  }
}

}  // namespace

namespace evrp::server {

GrpcEvrpService::GrpcEvrpService(Evrp* evrp) : evrp_(evrp) {}

grpc::Status GrpcEvrpService::Record(
    grpc::ServerContext*,
    const google::protobuf::Struct* request,
    evrp::v1::sdk::StatusCode* response) {
  auto settings = std::make_shared<MemorySetting>();
  std::map<std::string, std::any> protoFields;
  evrp::sdk::fromProto(protoFields, *request);
  mergeIntoMemorySetting(*settings, protoFields);
  settings->insert("program", std::string("evrp-grpc"));
  settings->insert("recording", true);
  settings->insert("playback", false);

  std::vector<DeviceKind> kinds =
      settings->get("kinds", std::vector<DeviceKind>{});
  if (kinds.empty()) {
    kinds = {DeviceKind::kTouchpad, DeviceKind::kTouchscreen,
             DeviceKind::kMouse, DeviceKind::kKeyboard};
  }
  settings->insert("kinds", std::move(kinds));

  const int code = evrp_->record(settings);
  response->set_code(code);
  if (code != 0) {
    response->set_message("record failed");
  }
  return grpc::Status::OK;
}

grpc::Status GrpcEvrpService::Replay(
    grpc::ServerContext*,
    const google::protobuf::Struct* request,
    evrp::v1::sdk::StatusCode* response) {
  auto settings = std::make_shared<MemorySetting>();
  std::map<std::string, std::any> protoFields;
  evrp::sdk::fromProto(protoFields, *request);
  mergeIntoMemorySetting(*settings, protoFields);
  settings->insert("program", std::string("evrp-grpc"));
  settings->insert("recording", false);
  settings->insert("playback", true);

  const std::string playbackPath =
      settings->get<std::string>("playbackPath", {});
  if (playbackPath.empty()) {
    response->set_code(1);
    response->set_message("playbackPath is required");
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "playbackPath is required");
  }

  const int code = evrp_->replay(settings);
  response->set_code(code);
  if (code != 0) {
    response->set_message("replay failed");
  }
  return grpc::Status::OK;
}

}  // namespace evrp::server
