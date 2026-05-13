#pragma once

#include <memory>
#include <string>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"

class ISetting;

namespace evrp::device::api {

class ICursorPosition;
class IDeviceKindsProvider;

class IClient {
 public:
  virtual ~IClient() = default;

  virtual IInputListener* inputListener() const = 0;
  virtual IPlayback* playback() const = 0;

  virtual IDeviceKindsProvider* deviceKindsProvider() const = 0;
  virtual ICursorPosition* cursorPosition() const = 0;

  virtual const std::string& serverAddress() const = 0;
};

std::unique_ptr<IClient> makeClient(const std::string& targetHostPort,
                                    const ISetting& discovery_settings);

}
