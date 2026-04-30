#pragma once

#include <any>
#include <string>
#include <type_traits>
#include <utility>

class ISetting {
 public:
  virtual ~ISetting() = default;

  virtual bool contains(const std::string& key) const = 0;
  virtual std::any get(const std::string& key) const = 0;
  virtual void insert(std::string key, std::any value) = 0;

  template <typename T>
  T get(const std::string& key, T defaultValue) const {
    std::any a = get(key);
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
    insert(std::move(key), std::any(std::forward<T>(value)));
  }
};
