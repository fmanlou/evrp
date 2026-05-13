#pragma once

#include "evrp/server/api/evrp.h"

namespace evrp::server {

class LocalEvrp final : public Evrp {
 public:
  LocalEvrp() = default;

  int record(std::shared_ptr<ISetting> settings) override;
  int replay(std::shared_ptr<ISetting> settings) override;
};

}  // namespace evrp::server
