#pragma once

#include <cstddef>
#include <cstdint>

namespace evrp::sdk {

inline constexpr int kDeviceDiscoveryUdpPort = 53508;
inline constexpr std::uint8_t kDeviceDiscoveryVersion = 1;

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

}  // namespace evrp::sdk
