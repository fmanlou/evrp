#include "evrp/sdk/setting/memorysetting.h"

bool MemorySetting::contains(const std::string& key) const {
  return values_.find(key) != values_.end();
}

std::any MemorySetting::get(const std::string& key) const {
  auto it = values_.find(key);
  if (it == values_.end()) return {};
  return it->second;
}

void MemorySetting::insert(std::string key, std::any value) {
  values_.insert_or_assign(std::move(key), std::move(value));
}
