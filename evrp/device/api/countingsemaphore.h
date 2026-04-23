#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace evrp {

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
  bool tryAcquire();

 private:
  std::mutex mu_;
  std::condition_variable cv_;
  std::ptrdiff_t count_{0};
};

}
