#pragma once

#include <utility>

namespace evrp::sdk {

// Runs a callable when leaving scope (function body, or earlier return).
template <typename F>
class ScopeGuard {
 public:
  explicit ScopeGuard(F &&f) : f_(std::forward<F>(f)) {}
  ~ScopeGuard() { f_(); }

  ScopeGuard(ScopeGuard &&other) noexcept : f_(std::move(other.f_)) {}
  ScopeGuard(const ScopeGuard &) = delete;
  ScopeGuard &operator=(const ScopeGuard &) = delete;
  ScopeGuard &operator=(ScopeGuard &&) = delete;

 private:
  F f_;
};

}  // namespace evrp::sdk
