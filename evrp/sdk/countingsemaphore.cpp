#include "evrp/sdk/countingsemaphore.h"

namespace evrp {

CountingSemaphore::CountingSemaphore() = default;

CountingSemaphore::~CountingSemaphore() = default;

void CountingSemaphore::release() {
  std::lock_guard<std::mutex> lock(mu_);
  ++count_;
  cv_.notify_one();
}

void CountingSemaphore::acquire() {
  std::unique_lock<std::mutex> lock(mu_);
  cv_.wait(lock, [this] { return count_ > 0; });
  --count_;
}

bool CountingSemaphore::tryAcquire() {
  std::lock_guard<std::mutex> lock(mu_);
  if (count_ == 0) {
    return false;
  }
  --count_;
  return true;
}

}
