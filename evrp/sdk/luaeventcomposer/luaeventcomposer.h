#pragma once

#include <string>
#include <vector>

#include "evrp/device/api/types.h"

class IEventComposer {
 public:
  virtual ~IEventComposer() = default;

  virtual int toEvents(const std::string& text,
                       std::vector<evrp::device::api::InputEvent>* events) = 0;
};

class LuaEventComposer final : public IEventComposer {
 public:
  int toEvents(const std::string& text,
               std::vector<evrp::device::api::InputEvent>* events) override;
};
