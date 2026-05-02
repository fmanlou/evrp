#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "evrp/sdk/setting/isetting.h"

class OverlaySetting final : public ISetting {
 public:
  explicit OverlaySetting(ISetting* top = nullptr);
  OverlaySetting(ISetting* top, std::initializer_list<const ISetting*> lowers);

  OverlaySetting(const OverlaySetting&) = delete;
  OverlaySetting& operator=(const OverlaySetting&) = delete;
  OverlaySetting(OverlaySetting&&) = delete;
  OverlaySetting& operator=(OverlaySetting&&) = delete;

  using ISetting::get;
  using ISetting::insert;

  void addLower(const ISetting* layer);
  void addLower(std::initializer_list<const ISetting*> layers);

  bool contains(const std::string& key) const override;
  std::any get(const std::string& key) const override;
  void insert(std::string key, std::any value) override;

 private:
  std::unique_ptr<ISetting> ownedTop_;
  ISetting* top_{};
  std::vector<const ISetting*> below_;
};
