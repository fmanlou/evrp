#include "evrp/sdk/setting/overlaysetting.h"

#include "evrp/sdk/setting/memorysetting.h"
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
