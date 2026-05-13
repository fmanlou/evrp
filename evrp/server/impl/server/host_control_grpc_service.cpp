#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/v1/server/host_control.grpc.pb.h"

#include "evrp/server/api/evrp.h"
#include "evrp/server/impl/server/host_control_grpc_service.h"

namespace {

using evrp::device::api::DeviceKind;
using evrp::device::api::toKind;

void applyHostSessionSettings(
    MemorySetting& out, const evrp::server::v1::HostSessionSettings& s) {
  out.insert("device", s.device());
  const std::string& lvl = s.log_level();
  out.insert("logLevel", logLevelFromString(lvl.empty() ? "info" : lvl));
  const std::string& kcf = s.keyboard_ctrl_c_filter();
  out.insert("keyboardCtrlCFilter", kcf.empty() ? std::string("ending") : kcf);
  if (s.discovery_port() != 0) {
    out.insert(evrp::sdk::kDeviceDiscoverySettingPort, s.discovery_port());
  }
  if (!s.discovery_link_mode().empty()) {
    out.insert(evrp::sdk::kDeviceDiscoverySettingLinkMode,
               s.discovery_link_mode());
  }
}

class HostControlServiceImpl final
    : public evrp::server::v1::HostControl::Service {
 public:
  grpc::Status Record(grpc::ServerContext*,
                      const evrp::server::v1::RecordRequest* request,
                      evrp::server::v1::RunResponse* response) override {
    auto settings = std::make_shared<MemorySetting>();
    applyHostSessionSettings(*settings, request->settings());
    settings->insert("program", std::string("evrp-grpc"));
    settings->insert("recording", true);
    settings->insert("playback", false);
    settings->insert("outputPath", request->output_path());

    std::vector<DeviceKind> kinds;
    kinds.reserve(static_cast<size_t>(request->kinds_size()));
    for (int i = 0; i < request->kinds_size(); ++i) {
      DeviceKind k = toKind(request->kinds(static_cast<int>(i)));
      if (k != DeviceKind::kUnspecified) {
        kinds.push_back(k);
      }
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
    if (request->playback_path().empty()) {
      response->set_exit_code(1);
      response->set_message("playback_path is required");
      return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "playback_path is required");
    }
    auto settings = std::make_shared<MemorySetting>();
    applyHostSessionSettings(*settings, request->settings());
    settings->insert("program", std::string("evrp-grpc"));
    settings->insert("recording", false);
    settings->insert("playback", true);
    settings->insert("playbackPath", request->playback_path());

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
