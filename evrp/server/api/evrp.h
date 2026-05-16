#pragma once

#include <memory>

#include "evrp/sdk/setting/isetting.h"

namespace evrp::server {

class Evrp {
 public:
  virtual ~Evrp() = default;

  virtual int record(std::shared_ptr<ISetting> settings) = 0;
  virtual int replay(std::shared_ptr<ISetting> settings) = 0;

  virtual bool isRecording() const = 0;
  virtual bool isReplaying() const = 0;

  virtual bool stopRecording() = 0;
  virtual bool stopReplay() = 0;
};

std::unique_ptr<Evrp> createClient();

}  // namespace evrp::server
