#include "evrp/device/api/server.h"

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"

#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "evrp/device/server/grpcinputdeviceservice.h"
#include "evrp/device/server/grpcinputlisten.h"
#include "evrp/device/server/grpcplaybackservice.h"
#include "evrp/ioc.h"
#include "logger.h"

namespace evrp::device::api {

int runDeviceServer(const std::string& listen_address, const evrp::Ioc& ioc) {
  IInputListener* input_listener = ioc.get<IInputListener*>();
  ICursorPosition* cursor_position = ioc.get<ICursorPosition*>();
  IInputDeviceKindsProvider* device_kinds_provider =
      ioc.get<IInputDeviceKindsProvider*>();
  IPlayback* playback = ioc.get<IPlayback*>();
  if (!input_listener || !cursor_position || !device_kinds_provider ||
      !playback) {
    logError("evrp-device: runDeviceServer requires non-null dependencies");
    return 1;
  }
  server::GrpcInputListenService listen_service(input_listener);
  server::GrpcInputDeviceService device_service(cursor_position,
                                                device_kinds_provider);
  server::GrpcPlaybackService playback_service(playback);

  grpc::EnableDefaultHealthCheckService(true);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&listen_service);
  builder.RegisterService(&device_service);
  builder.RegisterService(&playback_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    logError("evrp-device: failed to listen on " + listen_address);
    return 1;
  }

  logInfo("evrp-device listening on " + listen_address);
  server->Wait();
  return 0;
}

}  // namespace evrp::device::api
