#include "evrp/server/impl/client/remoteevrp.h"

#include <utility>

#include <gflags/gflags.h>
#include <google/protobuf/struct.pb.h>

#include "evrp/device/internal/discovery/devicediscovery.h"
#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/sessionclient.h"
#include "evrp/sdk/tofromproto.h"
#include "evrp/v1/server/service/evrp.grpc.pb.h"

DECLARE_string(device);
DECLARE_int32(discovery_port);
DECLARE_string(discovery_link_mode);

namespace evrp::server {

namespace {

std::shared_ptr<grpc::Channel> resolveHostGrpcChannel() {
  MemorySetting discoveryOpts;
  discoveryOpts.insert("device", std::string(FLAGS_device));
  discoveryOpts.insert(evrp::sdk::kDeviceDiscoverySettingPort,
                       FLAGS_discovery_port);
  discoveryOpts.insert(evrp::sdk::kDeviceDiscoverySettingLinkMode,
                       FLAGS_discovery_link_mode);

  std::string device = FLAGS_device;
  if (evrp::device::api::useUdpDeviceDiscovery(device)) {
    const std::unique_ptr<evrp::device::api::IUdpDeviceDiscoverer> discoverer =
        evrp::device::api::createUdpDeviceDiscoverer(discoveryOpts);
    for (const std::string& target : discoverer->discoverGrpcTargets()) {
      std::shared_ptr<grpc::Channel> ch =
          evrp::sdk::makeGrpcClientChannel(target);
      if (ch) {
        return ch;
      }
    }
    logError(
        "createClient: UDP discovery found no reachable evrp gRPC targets "
        "(same defaults as --device / discovery flags).");
    return nullptr;
  }

  return evrp::sdk::makeGrpcClientChannel(device);
}

}  // namespace

std::unique_ptr<Evrp> createClient() {
  std::shared_ptr<grpc::Channel> ch = resolveHostGrpcChannel();
  if (!ch) {
    return nullptr;
  }
  return std::make_unique<RemoteEvrp>(std::move(ch));
}

RemoteEvrp::RemoteEvrp(std::shared_ptr<grpc::Channel> channel)
    : channel_(std::move(channel)) {}

int RemoteEvrp::record(std::shared_ptr<ISetting> settings) {
  if (!settings) {
    logError("settings is null.");
    return 1;
  }
  google::protobuf::Struct req;
  evrp::sdk::toProto(settings->snapshot(), &req);

  grpc::ClientContext ctx;
  evrp::v1::sdk::StatusCode resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->Record(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.Record RPC failed: {}", st.error_message());
    return 1;
  }
  return resp.code();
}

int RemoteEvrp::replay(std::shared_ptr<ISetting> settings) {
  if (!settings) {
    logError("settings is null.");
    return 1;
  }
  google::protobuf::Struct req;
  evrp::sdk::toProto(settings->snapshot(), &req);

  grpc::ClientContext ctx;
  evrp::v1::sdk::StatusCode resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->Replay(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.Replay RPC failed: {}", st.error_message());
    return 1;
  }
  return resp.code();
}

}  // namespace evrp::server
