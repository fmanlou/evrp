#include "evrp/server/impl/server/grpc/grpcevrpservice.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <google/protobuf/empty.pb.h>
#include <google/protobuf/struct.pb.h>

#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/tofromproto.h"
#include "evrp/sdk/sessioncheck.h"
#include "evrp/server/api/evrp.h"
#include "evrp/server/impl/server/grpc/grpcsettingkey.h"

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

GrpcEvrpService::GrpcEvrpService(Evrp* evrp,
                                  evrp::session::SessionRegistry& clientSessions)
    : evrp_(evrp), clientSessions_(clientSessions) {}

grpc::Status GrpcEvrpService::requireEvrpClientSession(
    grpc::ServerContext* context) {
  return evrp::session::requireBusinessSession(context, clientSessions_);
}

grpc::Status GrpcEvrpService::Record(
    grpc::ServerContext* context,
    const google::protobuf::Struct* request,
    evrp::v1::sdk::StatusCode* response) {
  if (grpc::Status st = requireEvrpClientSession(context); !st.ok()) {
    return st;
  }
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

  if (context != nullptr) {
    settings->insert(kUnaryRpcServerContextSettingKey, context);
  }

  const int code = evrp_->record(settings);
  response->set_code(code);
  if (code != 0) {
    response->set_message("record failed");
  }
  return grpc::Status::OK;
}

grpc::Status GrpcEvrpService::Replay(
    grpc::ServerContext* context,
    const google::protobuf::Struct* request,
    evrp::v1::sdk::StatusCode* response) {
  if (grpc::Status st = requireEvrpClientSession(context); !st.ok()) {
    return st;
  }
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

  if (context != nullptr) {
    settings->insert(kUnaryRpcServerContextSettingKey, context);
  }

  const int code = evrp_->replay(settings);
  response->set_code(code);
  if (code != 0) {
    response->set_message("replay failed");
  }
  return grpc::Status::OK;
}

grpc::Status GrpcEvrpService::IsRecording(
    grpc::ServerContext* context,
    const google::protobuf::Empty*,
    evrp::v1::server::BoolPayload* response) {
  if (grpc::Status st = requireEvrpClientSession(context); !st.ok()) {
    return st;
  }
  response->set_value(evrp_->isRecording());
  return grpc::Status::OK;
}

grpc::Status GrpcEvrpService::IsReplaying(
    grpc::ServerContext* context,
    const google::protobuf::Empty*,
    evrp::v1::server::BoolPayload* response) {
  if (grpc::Status st = requireEvrpClientSession(context); !st.ok()) {
    return st;
  }
  response->set_value(evrp_->isReplaying());
  return grpc::Status::OK;
}

grpc::Status GrpcEvrpService::StopRecording(
    grpc::ServerContext* context,
    const google::protobuf::Empty*,
    evrp::v1::sdk::StatusCode* response) {
  if (grpc::Status st = requireEvrpClientSession(context); !st.ok()) {
    return st;
  }
  const bool ok = evrp_->stopRecording();
  response->set_code(ok ? 0 : 1);
  if (!ok) {
    response->set_message("stopRecording failed");
  }
  return grpc::Status::OK;
}

grpc::Status GrpcEvrpService::StopReplay(
    grpc::ServerContext* context,
    const google::protobuf::Empty*,
    evrp::v1::sdk::StatusCode* response) {
  if (grpc::Status st = requireEvrpClientSession(context); !st.ok()) {
    return st;
  }
  const bool ok = evrp_->stopReplay();
  response->set_code(ok ? 0 : 1);
  if (!ok) {
    response->set_message("stopReplay failed");
  }
  return grpc::Status::OK;
}

}  // namespace evrp::server
