#pragma once

#include "evrp/sdk/types.h"

class IRawEventWriter {
 public:
  virtual ~IRawEventWriter() = default;
  virtual bool writeRaw(evrp::sdk::DeviceKind device,
                        unsigned short type, unsigned short code, int value) = 0;
};
