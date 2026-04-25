#include "evrp/device/impl/server/localinputdevicekindsprovider.h"

#include <sys/stat.h>

#include <string>

#include "evrp/device/api/types.h"
#include "evdev.h"
#include "inputdevice.h"
#include "logger.h"

namespace evrp::device::server {

namespace {

struct EvdevNodeStats {
  int characterDevices = 0;
  int capsReadable = 0;
  int capsUnreadable = 0;
};

EvdevNodeStats scanEvdevNodesForProbe() {
  EvdevNodeStats s;
  for (int i = 0; i < 32; ++i) {
    const std::string path = "/dev/input/event" + std::to_string(i);
    struct stat st {};
    if (stat(path.c_str(), &st) != 0 || !S_ISCHR(st.st_mode)) {
      continue;
    }
    s.characterDevices++;
    Capabilities caps{};
    if (openAndGetCapabilities(path.c_str(), &caps)) {
      s.capsReadable++;
    } else {
      s.capsUnreadable++;
    }
  }
  return s;
}

std::string formatKindList(const std::vector<api::DeviceKind>& kinds) {
  std::string s;
  for (size_t i = 0; i < kinds.size(); ++i) {
    if (i > 0) {
      s += ", ";
    }
    s += api::deviceKindLabel(kinds[i]);
  }
  return s;
}

}  

std::vector<api::DeviceKind> LocalInputDeviceKindsProvider::kinds() {
  static const api::DeviceKind k_order[] = {
      api::DeviceKind::kTouchpad,
      api::DeviceKind::kTouchscreen,
      api::DeviceKind::kMouse,
      api::DeviceKind::kKeyboard,
  };
  std::vector<api::DeviceKind> out;
  for (api::DeviceKind k : k_order) {
    if (!findDevicePath(k).empty()) {
      out.push_back(k);
    }
  }

  const EvdevNodeStats ev = scanEvdevNodesForProbe();
  if (out.empty()) {
    logInfo(
        "evrp-device: input capabilities probe finished: no supported kinds "
        "(touchpad/touchscreen/mouse/keyboard). /dev/input/event0-31: {} "
        "character device node(s); {} opened and EVIOC* capability read "
        "succeeded; {} failed open or cap query (often EACCES: add user to "
        "group \"input\" or run with access to /dev/input/*). If "
        "capsReadable>0 but still no kinds, no node matched "
        "keyboard/mouse/touchpad/touchscreen heuristics.",
        ev.characterDevices,
        ev.capsReadable,
        ev.capsUnreadable);
  } else {
    logInfo(
        "evrp-device: input capabilities probe finished: supported kinds {} "
        "({} kind(s)); /dev/input/event0-31: {} node(s), {} with readable caps.",
        formatKindList(out),
        out.size(),
        ev.characterDevices,
        ev.capsReadable);
  }
  return out;
}

}
