#pragma once

#include <string>

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/api/playback.h"

namespace evrp::device::api {

int runDeviceServer(const std::string& listen_address,
                    IInputListener& input_listener, ICursorPosition& cursor_position,
                    IInputDeviceKindsProvider& device_kinds_provider, IPlayback& playback);

}  // namespace evrp::device::api
