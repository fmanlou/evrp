#pragma once

#include <memory>

#include "evrp/sdk/setting/isetting.h"

namespace evrp::server {

class Evrp {
 public:
  virtual ~Evrp() = default;

  virtual int record(std::shared_ptr<ISetting> settings) = 0;
  virtual int replay(std::shared_ptr<ISetting> settings) = 0;
};

std::unique_ptr<Evrp> createClient();

}  // namespace evrp::server
