#pragma once

#include <any>
#include <map>
#include <string>
#include <type_traits>
#include <utility>

class StringKeyStoreCore {
 public:
  StringKeyStoreCore() = default;
  virtual ~StringKeyStoreCore() = default;

  virtual bool contains(const std::string& key) const = 0;
  virtual std::any get(const std::string& key) const = 0;
  virtual void insert(std::string key, std::any value) = 0;
};

class StringKeyStore {
 public:
  explicit StringKeyStore(StringKeyStoreCore& core) : core_(core) {}

  bool contains(const std::string& key) const { return core_.contains(key); }

  std::any get(const std::string& key) const { return core_.get(key); }

  void insert(std::string key, std::any value) {
    core_.insert(std::move(key), std::move(value));
  }

  StringKeyStoreCore& core() { return core_; }
  const StringKeyStoreCore& core() const { return core_; }

  template <typename T>
  T get(const std::string& key, T defaultValue) const {
    std::any a = core_.get(key);
    if (!a.has_value()) return defaultValue;
    if (auto* p = std::any_cast<T>(&a)) return *p;
    return defaultValue;
  }

  template <typename T>
  T get(const std::string& key) const {
    return get(key, T{});
  }

  template <typename T,
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::any>>>
  void insert(std::string key, T&& value) {
    core_.insert(std::move(key), std::any(std::forward<T>(value)));
  }

 private:
  StringKeyStoreCore& core_;
};

class MapStringKeyStore final : public StringKeyStoreCore {
 public:
  MapStringKeyStore() = default;

  bool contains(const std::string& key) const override {
    return values_.find(key) != values_.end();
  }

  std::any get(const std::string& key) const override {
    auto it = values_.find(key);
    if (it == values_.end()) return {};
    return it->second;
  }

  void insert(std::string key, std::any value) override {
    values_.insert_or_assign(std::move(key), std::move(value));
  }

 private:
  std::map<std::string, std::any> values_;
};
