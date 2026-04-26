#include "evrp/device/impl/server/localinputlistener.h"

#include <linux/input-event-codes.h>

#include <cerrno>
#include <cstring>
#include <unordered_set>
#include <vector>

#include "evrp/sdk/evdev.h"
#include "evrp/sdk/eventformat.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/inputdevice.h"
#include "evrp/sdk/logger.h"

namespace evrp::device::server {

namespace {

api::InputEvent toApiInputEvent(api::DeviceKind device, const Event& ev) {
  api::InputEvent e;
  e.device = device;
  e.timeSec = ev.sec;
  e.timeUsec = ev.usec;
  e.type = static_cast<uint32_t>(ev.type);
  e.code = static_cast<uint32_t>(ev.code);
  e.value = ev.value;
  return e;
}

}  

void LocalInputListener::dispose() {
  cancelListening();
  std::lock_guard<std::mutex> lock(mu_);
  disposed_ = true;
}

LocalInputListener::~LocalInputListener() { dispose(); }

std::string LocalInputListener::listenDevicesSummary() const {
  std::string list;
  for (const auto& d : devices_) {
    if (!list.empty()) {
      list += ", ";
    }
    list += api::deviceKindLabel(d.kind);
    list += "=";
    list += d.path;
  }
  return list;
}

void LocalInputListener::closeDevices() {
  if (!devices_.empty()) {
    logInfo("LocalInputListener: stop listening ({}): {}",
            devices_.size(),
            listenDevicesSummary());
  }
  for (auto& d : devices_) {
    if (d.fd >= 0) {
      fs_.closeFd(d.fd);
    }
    d.fd = -1;
  }
  devices_.clear();
  pollReadyIndices_.clear();
}

bool LocalInputListener::startListening(
    const std::vector<api::DeviceKind>& kinds) {
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_) {
    return false;
  }
  if (listeningActive_) {
    logWarn(
        "LocalInputListener: startListening refused: listeningActive_ is "
        "still true (StopRecording may not have completed yet)");
    return false;
  }

  std::unordered_set<std::string> opened_paths;
  std::vector<TrackedDevice> new_devices;

  for (api::DeviceKind k : kinds) {
    if (k == api::DeviceKind::kUnspecified) {
      continue;
    }
    const std::string kindLabel = api::deviceKindLabel(k);
    const std::vector<std::string> paths = findAllDevicePaths(k);
    if (paths.empty()) {
      logWarn(
          "LocalInputListener: kind {} has no matching /dev/input node "
          "(findAllDevicePaths empty)",
          kindLabel);
      continue;
    }
    for (const std::string& path : paths) {
      if (new_devices.size() >= 32) {
        break;
      }
      if (!opened_paths.insert(path).second) {
        logInfo(
            "LocalInputListener: skip kind {} path {} (same node already "
            "opened for this session)",
            kindLabel,
            path);
        continue;
      }
      int fd = fs_.openReadOnly(path.c_str(), true);
      if (fd < 0) {
        logWarn(
            "LocalInputListener: kind {} path {} open(O_RDONLY) failed: {}",
            kindLabel,
            path,
            std::strerror(errno));
        continue;
      }
      new_devices.push_back(TrackedDevice{fd, k, path});
    }
    if (new_devices.size() >= 32) {
      break;
    }
  }

  if (new_devices.empty()) {
    logWarn(
        "LocalInputListener: startListening failed: no evdev fds opened for "
        "requested kind(s) (see per-kind messages above)");
    return false;
  }

  devices_ = std::move(new_devices);
  listeningActive_ = true;
  logInfo("LocalInputListener: start listening ({}): {}",
          devices_.size(),
          listenDevicesSummary());
  return true;
}

std::vector<api::InputEvent> LocalInputListener::readInputEvents() {
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_) {
    return {};
  }
  if (!listeningActive_ || devices_.empty()) {
    return {};
  }

  size_t n = devices_.size();
  if (n > 32) {
    n = 32;
  }

  if (pollReadyIndices_.empty()) {
    return {};
  }

  std::vector<api::InputEvent> out;
  Event evbuf[64];

  for (size_t i : pollReadyIndices_) {
    if (i >= n) {
      continue;
    }
    if (!listeningActive_) {
      return out;
    }
    int ne = readEvents(devices_[i].fd, evbuf, 64);
    if (ne <= 0) {
      continue;
    }
    for (int j = 0; j < ne; ++j) {
      if (!listeningActive_) {
        return out;
      }
      if (evbuf[j].type == EV_SYN) {
        continue;
      }
      logDebug("LocalInputListener: evdev={} {}",
               devices_[i].path,
               formatEventLine(devices_[i].kind, evbuf[j], 0));
      out.push_back(toApiInputEvent(devices_[i].kind, evbuf[j]));
    }
  }
  pollReadyIndices_.clear();
  return out;
}

bool LocalInputListener::waitForInputEvent(int timeoutMs) {
  if (!listeningActive_ || disposed_) {
    return false;
  }
  if (timeoutMs < 0) {
    return false;
  }
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_ || !listeningActive_ || devices_.empty()) {
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
  int ret = fs_.pollFds(fds, n_poll, timeoutMs, ready);
  if (!listeningActive_ || disposed_) {
    return false;
  }
  if (ret <= 0) {
    return false;
  }
  for (int j = 0; j < n_poll; ++j) {
    if (ready[j]) {
      pollReadyIndices_.insert(static_cast<size_t>(j));
    }
  }
  return !pollReadyIndices_.empty();
}

bool LocalInputListener::isListening() const { return listeningActive_; }

void LocalInputListener::cancelListening() {
  listeningActive_ = false;
  std::lock_guard<std::mutex> lock(mu_);
  if (disposed_) {
    return;
  }
  closeDevices();
}

}
