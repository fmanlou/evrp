#pragma once

#include <any>
#include <map>
#include <typeindex>
#include <typeinfo>

namespace evrp {

class Ioc {
 public:
  template <typename T>
  void emplace(T* p) {
    byType_[std::type_index(typeid(T*))] = std::make_any<T*>(p);
  }

  template <typename T>
  T* get() const {
    using Ptr = T*;
    auto it = byType_.find(std::type_index(typeid(Ptr)));
    if (it == byType_.end()) {
      return nullptr;
    }
    const std::any& a = it->second;
    if (!a.has_value()) {
      return nullptr;
    }
    if (const Ptr* holder = std::any_cast<Ptr>(&a)) {
      return *holder;
    }
    return nullptr;
  }

 private:
  std::map<std::type_index, std::any> byType_;
};

}
