#pragma once

#include <utility>

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

template <typename F>
ScopeGuard<F> make_scope_guard(F &&f) {
  return ScopeGuard<F>(std::forward<F>(f));
}
