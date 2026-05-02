#pragma once

namespace evrp::sdk {

inline constexpr int kDeviceDiscoveryUdpPort = 53508;
inline constexpr char kDeviceDiscoverySettingPort[] = "discovery_port";
inline constexpr char kDeviceDiscoverySettingLinkMode[] = "discovery_link_mode";
inline constexpr char kDeviceDiscoverySettingGrpcListenPort[] =
    "discovery_grpc_listen_port";

}
