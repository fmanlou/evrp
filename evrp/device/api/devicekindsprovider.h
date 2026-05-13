#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IDeviceKindsProvider {
 public:
  virtual ~IDeviceKindsProvider() = default;

  virtual std::vector<DeviceKind> kinds() = 0;

  /** Fills supported kinds; returns false when unavailable (e.g. remote RPC failure). */
  virtual bool queryKinds(std::vector<DeviceKind>* out) {
    if (!out) {
      return false;
    }
    *out = kinds();
    return true;
  }
};

}
