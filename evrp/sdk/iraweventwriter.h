#pragma once

#include "evrp/device/api/types.h"

class IRawEventWriter {
 public:
  virtual ~IRawEventWriter() = default;
  virtual bool writeRaw(evrp::device::api::DeviceKind device,
                        unsigned short type, unsigned short code, int value) = 0;
};
