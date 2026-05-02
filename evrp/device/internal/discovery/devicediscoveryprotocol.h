#pragma once

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>

namespace evrp::sdk {

inline constexpr int kDeviceDiscoveryUdpPort = 53508;
inline constexpr char kDeviceDiscoverySettingPort[] = "discovery_port";
inline constexpr char kDeviceDiscoverySettingLinkMode[] = "discovery_link_mode";
inline constexpr char kDeviceDiscoveryMulticastIpv4[] = "239.76.82.80";
inline constexpr std::uint8_t kDeviceDiscoveryVersion = 1;

enum class DiscoveryLinkMode : std::uint8_t { kMulticast = 0, kBroadcast = 1 };

inline bool tryParseDiscoveryLinkMode(const std::string& s,
                                      DiscoveryLinkMode* out) {
  if (!out) {
    return false;
  }
  std::string lower;
  lower.reserve(s.size());
  for (char c : s) {
    lower.push_back(static_cast<char>(
        std::tolower(static_cast<unsigned char>(c))));
  }
  if (lower == "multicast") {
    *out = DiscoveryLinkMode::kMulticast;
    return true;
  }
  if (lower == "broadcast") {
    *out = DiscoveryLinkMode::kBroadcast;
    return true;
  }
  return false;
}

#pragma pack(push, 1)
struct DeviceDiscoveryRequest {
  char magic[4];
  std::uint8_t version;
  std::uint8_t reserved[3];
};
struct DeviceDiscoveryResponse {
  char magic[4];
  std::uint8_t version;
  std::uint8_t reserved;
  std::uint16_t grpc_port_be;
};
#pragma pack(pop)

static_assert(sizeof(DeviceDiscoveryRequest) == 8);
static_assert(sizeof(DeviceDiscoveryResponse) == 8);

inline void fillDeviceDiscoveryRequest(DeviceDiscoveryRequest* r) {
  r->magic[0] = 'E';
  r->magic[1] = 'V';
  r->magic[2] = 'R';
  r->magic[3] = 'P';
  r->version = kDeviceDiscoveryVersion;
  r->reserved[0] = 0;
  r->reserved[1] = 0;
  r->reserved[2] = 0;
}

inline bool isValidDeviceDiscoveryRequest(const DeviceDiscoveryRequest& r) {
  return r.magic[0] == 'E' && r.magic[1] == 'V' && r.magic[2] == 'R' &&
         r.magic[3] == 'P' && r.version == kDeviceDiscoveryVersion;
}

inline bool isValidDeviceDiscoveryResponse(const DeviceDiscoveryResponse& r) {
  return r.magic[0] == 'E' && r.magic[1] == 'V' && r.magic[2] == 'R' &&
         r.magic[3] == 'P' && r.version == kDeviceDiscoveryVersion;
}

}
