#include "evrp/device/server/localinputlistener.h"

#include <linux/input-event-codes.h>

#include <unordered_set>

#include "deviceid.h"
#include "evdev.h"
#include "inputdevice.h"

namespace evrp::device::server {

namespace {

DeviceId device_id_from_api_kind(api::DeviceKind k) {
  switch (k) {
    case api::DeviceKind::kTouchpad:
      return DeviceId::Touchpad;
    case api::DeviceKind::kTouchscreen:
      return DeviceId::Touchscreen;
    case api::DeviceKind::kMouse:
      return DeviceId::Mouse;
    case api::DeviceKind::kKeyboard:
      return DeviceId::Keyboard;
    default:
      return DeviceId::Unknown;
  }
}

api::InputEvent to_api_input_event(api::DeviceKind device, const Event& ev) {
  api::InputEvent e;
  e.device = device;
  e.time_sec = ev.sec;
  e.time_usec = ev.usec;
  e.type = static_cast<uint32_t>(ev.type);
  e.code = static_cast<uint32_t>(ev.code);
  e.value = ev.value;
  return e;
}

}  // namespace

void LocalInputListener::dispose() {
  cancel_listening();
  std::lock_guard<std::mutex> lock(mu_);
  disposed_ = true;
}

LocalInputListener::~LocalInputListener() { dispose(); }

void LocalInputListener::close_devices() {
  for (auto& d : devices_) {
    if (d.fd >= 0) {
      fs_.close_fd(d.fd);
    }
    d.fd = -1;
  }
  devices_.clear();
  poll_ready_indices_.clear();
}

bool LocalInputListener::start_listening(
    const std::vector<api::DeviceKind>& kinds) {
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_) {
    return false;
  }
  if (listening_active_) {
    return false;
  }

  std::unordered_set<std::string> opened_paths;
  std::vector<TrackedDevice> new_devices;

  for (api::DeviceKind k : kinds) {
    if (k == api::DeviceKind::kUnspecified) {
      continue;
    }
    DeviceId id = device_id_from_api_kind(k);
    if (id == DeviceId::Unknown) {
      continue;
    }
    std::string path = find_device_path(id);
    if (path.empty()) {
      continue;
    }
    if (!opened_paths.insert(path).second) {
      continue;
    }
    int fd = fs_.open_read_only(path.c_str(), true);
    if (fd < 0) {
      continue;
    }
    new_devices.push_back(TrackedDevice{fd, k});
    if (new_devices.size() >= 32) {
      break;
    }
  }

  if (new_devices.empty()) {
    return false;
  }

  devices_ = std::move(new_devices);
  listening_active_ = true;
  return true;
}

std::vector<api::InputEvent> LocalInputListener::read_input_events() {
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_) {
    return {};
  }
  if (!listening_active_ || devices_.empty()) {
    return {};
  }

  size_t n = devices_.size();
  if (n > 32) {
    n = 32;
  }

  if (poll_ready_indices_.empty()) {
    return {};
  }

  std::vector<api::InputEvent> out;
  Event evbuf[64];

  for (size_t i : poll_ready_indices_) {
    if (i >= n) {
      continue;
    }
    if (!listening_active_) {
      return out;
    }
    int ne = read_events(devices_[i].fd, evbuf, 64);
    if (ne <= 0) {
      continue;
    }
    for (int j = 0; j < ne; ++j) {
      if (!listening_active_) {
        return out;
      }
      if (evbuf[j].type == EV_SYN) {
        continue;
      }
      out.push_back(to_api_input_event(devices_[i].kind, evbuf[j]));
    }
  }
  poll_ready_indices_.clear();
  return out;
}

bool LocalInputListener::wait_for_input_event(int timeout_ms) {
  if (!listening_active_ || disposed_) {
    return false;
  }
  if (timeout_ms < 0) {
    return false;
  }
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_ || !listening_active_ || devices_.empty()) {
    return false;
  }
  int fds[32];
  size_t n = devices_.size();
  if (n > 32) {
    n = 32;
  }
  int n_poll = static_cast<int>(n);
  for (size_t i = 0; i < n; ++i) {
    fds[i] = devices_[i].fd;
  }
  bool ready[32]{};
  int ret = fs_.poll_fds(fds, n_poll, timeout_ms, ready);
  if (!listening_active_ || disposed_) {
    return false;
  }
  if (ret <= 0) {
    return false;
  }
  for (int j = 0; j < n_poll; ++j) {
    if (ready[j]) {
      poll_ready_indices_.insert(static_cast<size_t>(j));
    }
  }
  return !poll_ready_indices_.empty();
}

bool LocalInputListener::is_listening() const { return listening_active_; }

void LocalInputListener::cancel_listening() {
  listening_active_ = false;
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_) {
    return;
  }
  close_devices();
}

}  // namespace evrp::device::server
