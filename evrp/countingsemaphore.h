#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace evrp {

// Counting semaphore using std::mutex and std::condition_variable (C++17).
class CountingSemaphore {
 public:
  CountingSemaphore();
  ~CountingSemaphore();

  CountingSemaphore(const CountingSemaphore&) = delete;
  CountingSemaphore& operator=(const CountingSemaphore&) = delete;
  CountingSemaphore(CountingSemaphore&&) = delete;
  CountingSemaphore& operator=(CountingSemaphore&&) = delete;

  void release();
  void acquire();
  bool try_acquire();

 private:
  std::mutex mu_;
  std::condition_variable cv_;
  std::ptrdiff_t count_{0};
};

}  // namespace evrp
