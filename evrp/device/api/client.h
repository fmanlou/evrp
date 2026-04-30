#pragma once

#include <memory>
#include <string>

#include "evrp/device/api/inputdeviceclient.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"

namespace evrp::device::api {

class IClient {
 public:
  virtual ~IClient() = default;

  virtual IInputListener* inputListener() const = 0;
  virtual IPlayback* playback() const = 0;
  virtual IInputDeviceClient* inputDevice() const = 0;

  virtual const std::string& serverAddress() const = 0;
};

std::unique_ptr<IClient> makeClient(const std::string& targetHostPort);

}
