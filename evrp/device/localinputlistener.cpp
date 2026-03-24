#include "evrp/device/localinputlistener.h"

#include <linux/input-event-codes.h>

#include <unordered_set>

#include "deviceid.h"
#include "evdev.h"
#include "inputdevice.h"

namespace evrp::device {

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

LocalInputListener::~LocalInputListener() {
  std::lock_guard<std::mutex> lock(mu_);
  listening_active_ = false;
  close_devices_unlocked();
}

void LocalInputListener::close_devices_unlocked() {
  for (auto& d : devices_) {
    if (d.fd >= 0) {
      fs_.close_fd(d.fd);
    }
    d.fd = -1;
  }
  devices_.clear();
}

bool LocalInputListener::start_listening(const std::vector<api::DeviceKind>& kinds) {
  std::lock_guard<std::mutex> lock(mu_);
  if (listening_active_.load()) {
    return false;
  }
  close_devices_unlocked();

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
  if (!listening_active_.load() || devices_.empty()) {
    return {};
  }

  size_t n = devices_.size();
  if (n > 32) {
    n = 32;
  }

  std::vector<int> fds;
  fds.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    fds.push_back(devices_[i].fd);
  }

  std::vector<api::InputEvent> out;
  Event evbuf[64];
  bool ready[32]{};

  int ret = fs_.poll_fds(fds.data(), static_cast<int>(fds.size()), 0, ready);
  if (ret <= 0) {
    return {};
  }

  for (size_t i = 0; i < n; ++i) {
    if (!ready[i]) {
      continue;
    }
    int ne = read_events(devices_[i].fd, evbuf, 64);
    if (ne <= 0) {
      continue;
    }
    for (int j = 0; j < ne; ++j) {
      if (evbuf[j].type == EV_SYN) {
        continue;
      }
      out.push_back(to_api_input_event(devices_[i].kind, evbuf[j]));
    }
  }
  return out;
}

void LocalInputListener::cancel_listening() {
  std::lock_guard<std::mutex> lock(mu_);
  listening_active_ = false;
  close_devices_unlocked();
}

}  // namespace evrp::device
