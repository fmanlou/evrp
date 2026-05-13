#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>

#include "evrp/sdk/setting/isetting.h"

class MemorySetting final : public ISetting {
 public:
  MemorySetting() = default;

  MemorySetting(const MemorySetting&) = delete;
  MemorySetting& operator=(const MemorySetting&) = delete;
  MemorySetting(MemorySetting&&) = delete;
  MemorySetting& operator=(MemorySetting&&) = delete;

  using ISetting::get;
  using ISetting::insert;

  bool contains(const std::string& key) const override;
  std::any get(const std::string& key) const override;
  void insert(std::string key, std::any value) override;
  std::vector<std::string> keys() const override;

 private:
  std::map<std::string, std::any> values_;
};
