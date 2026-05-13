#include "evrp/sdk/setting/overlaysetting.h"

#include "evrp/sdk/setting/memorysetting.h"
#include <unordered_set>
#include <utility>

OverlaySetting::OverlaySetting(ISetting* top) {
  if (top) {
    top_ = top;
  } else {
    ownedTop_ = std::make_unique<MemorySetting>();
    top_ = ownedTop_.get();
  }
}

OverlaySetting::OverlaySetting(ISetting* top,
                               std::initializer_list<const ISetting*> lowers)
    : OverlaySetting(top) {
  addLower(lowers);
}

void OverlaySetting::addLower(const ISetting* layer) {
  if (!layer) {
    return;
  }
  below_.push_back(layer);
}

void OverlaySetting::addLower(std::initializer_list<const ISetting*> layers) {
  below_.reserve(below_.size() + layers.size());
  for (const ISetting* layer : layers) {
    if (layer) {
      below_.push_back(layer);
    }
  }
}

bool OverlaySetting::contains(const std::string& key) const {
  if (top_->contains(key)) {
    return true;
  }
  for (const ISetting* layer : below_) {
    if (layer->contains(key)) {
      return true;
    }
  }
  return false;
}

std::any OverlaySetting::get(const std::string& key) const {
  if (top_->contains(key)) {
    return top_->get(key);
  }
  for (const ISetting* layer : below_) {
    if (layer->contains(key)) {
      return layer->get(key);
    }
  }
  return {};
}

void OverlaySetting::insert(std::string key, std::any value) {
  top_->insert(std::move(key), std::move(value));
}

std::vector<std::string> OverlaySetting::keys() const {
  std::vector<std::string> result = top_->keys();
  std::unordered_set<std::string> seen(result.begin(), result.end());
  for (const ISetting* layer : below_) {
    if (!layer) {
      continue;
    }
    for (const std::string& k : layer->keys()) {
      if (seen.insert(k).second) {
        result.push_back(k);
      }
    }
  }
  return result;
}

std::map<std::string, std::any> OverlaySetting::snapshot() const {
  std::map<std::string, std::any> out;
  for (const std::string& k : keys()) {
    out.emplace(k, get(k));
  }
  return out;
}
