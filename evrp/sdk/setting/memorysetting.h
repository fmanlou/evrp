#pragma once

#include <any>
#include <map>
#include <string>

#include "evrp/sdk/setting/isetting.h"

class MemorySetting final : public ISetting {
 public:
  MemorySetting() = default;

  using ISetting::get;
  using ISetting::insert;

  bool contains(const std::string& key) const override;
  std::any get(const std::string& key) const override;
  void insert(std::string key, std::any value) override;

 private:
  std::map<std::string, std::any> values_;
};
