#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "argparser.h"
#include "evrp/device/api/types.h"
#include "filesystem.h"

namespace evrp::device::api {
class IClient;
}

struct RecordTarget {
  int fd;
  evrp::device::api::DeviceKind kind;
  std::string path;
};

class Record {
 public:
  explicit Record(const RunOptions &options);
  int run();
  /// Records via evrp-device (InputListen gRPC); requires a connected client.
  int runWithDeviceClient(evrp::device::api::IClient *client);

 private:
  std::vector<RecordTarget> collectTargets();
  void closeTargets();
  void recordEvents();

  RunOptions options_;
  FileSystem fs_;
  std::vector<RecordTarget> targets_;
};
