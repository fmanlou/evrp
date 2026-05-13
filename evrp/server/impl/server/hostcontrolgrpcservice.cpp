#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "evrp/sdk/tofromstring.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/tofromproto.h"
#include "evrp/v1/server/hostcontrol.grpc.pb.h"

#include "evrp/server/api/evrp.h"
#include "evrp/server/impl/server/hostcontrolgrpcservice.h"

namespace {

using evrp::sdk::DeviceKind;
using evrp::sdk::toKind;

void mergeIntoMemorySetting(
    MemorySetting& settings,
    const std::map<std::string, std::any>& fields) {
  for (const auto& entry : fields) {
    settings.insert(entry.first, entry.second);
  }
}

class HostControlServiceImpl final
    : public evrp::server::v1::HostControl::Service {
 public:
  grpc::Status Record(grpc::ServerContext*,
                      const evrp::server::v1::RecordRequest* request,
                      evrp::server::v1::RunResponse* response) override {
    auto settings = std::make_shared<MemorySetting>();
    std::map<std::string, std::any> protoFields;
    evrp::sdk::fromProto(protoFields, request->settings());
    mergeIntoMemorySetting(*settings, protoFields);
    settings->insert("program", std::string("evrp-grpc"));
    settings->insert("recording", true);
    settings->insert("playback", false);
    if (!request->output_path().empty()) {
      settings->insert("outputPath", request->output_path());
    }

    std::vector<DeviceKind> kinds;
    if (request->kinds_size() > 0) {
      kinds.reserve(static_cast<size_t>(request->kinds_size()));
      for (int i = 0; i < request->kinds_size(); ++i) {
        DeviceKind k = toKind(request->kinds(static_cast<int>(i)));
        if (k != DeviceKind::kUnspecified) {
          kinds.push_back(k);
        }
      }
    }
    if (kinds.empty()) {
      kinds = settings->get("kinds", std::vector<DeviceKind>{});
    }
    if (kinds.empty()) {
      kinds = {DeviceKind::kTouchpad, DeviceKind::kTouchscreen,
               DeviceKind::kMouse, DeviceKind::kKeyboard};
    }
    settings->insert("kinds", std::move(kinds));

    const int code = ::evrp::server::record(settings);
    response->set_exit_code(code);
    if (code != 0) {
      response->set_message("record failed");
    }
    return grpc::Status::OK;
  }

  grpc::Status Replay(grpc::ServerContext*,
                      const evrp::server::v1::ReplayRequest* request,
                      evrp::server::v1::RunResponse* response) override {
    auto settings = std::make_shared<MemorySetting>();
    std::map<std::string, std::any> protoFields;
    evrp::sdk::fromProto(protoFields, request->settings());
    mergeIntoMemorySetting(*settings, protoFields);
    settings->insert("program", std::string("evrp-grpc"));
    settings->insert("recording", false);
    settings->insert("playback", true);
    if (!request->playback_path().empty()) {
      settings->insert("playbackPath", request->playback_path());
    }

    const std::string playbackPath =
        settings->get<std::string>("playbackPath", {});
    if (playbackPath.empty()) {
      response->set_exit_code(1);
      response->set_message("playback_path is required");
      return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "playback_path is required");
    }

    const int code = ::evrp::server::replay(settings);
    response->set_exit_code(code);
    if (code != 0) {
      response->set_message("replay failed");
    }
    return grpc::Status::OK;
  }
};

}  // namespace

struct evrp::server::HostControlGrpcService::Impl {
  std::unique_ptr<evrp::server::v1::HostControl::Service> service;

  Impl() : service(std::make_unique<HostControlServiceImpl>()) {}
};

evrp::server::HostControlGrpcService::HostControlGrpcService()
    : impl_(std::make_unique<Impl>()) {}

evrp::server::HostControlGrpcService::~HostControlGrpcService() = default;

evrp::server::HostControlGrpcService::HostControlGrpcService(
    HostControlGrpcService&&) noexcept = default;

evrp::server::HostControlGrpcService& evrp::server::HostControlGrpcService::operator=(
    HostControlGrpcService&&) noexcept = default;

grpc::Service* evrp::server::HostControlGrpcService::grpc_service() {
  return impl_->service.get();
}
