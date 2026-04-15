#pragma once

#include <any>
#include <map>
#include <typeindex>
#include <typeinfo>

namespace evrp {

class Ioc {
 public:
  template <typename Ptr>
  void emplace(Ptr p) {
    by_type_[std::type_index(typeid(Ptr))] = std::make_any<Ptr>(p);
  }

  template <typename Ptr>
  Ptr get() const {
    auto it = by_type_.find(std::type_index(typeid(Ptr)));
    if (it == by_type_.end()) {
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
  std::map<std::type_index, std::any> by_type_;
};

}  // namespace evrp
