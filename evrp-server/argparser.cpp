#include "evrp-server/argparser.h"

#include <gflags/gflags.h>

#include <string>
#include <vector>

#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/sdk/logger.h"

DEFINE_string(
    log_level, "info",
    "evrp-server log level: error|warn|info|debug|trace.");
DEFINE_string(
    listen, "",
    "Listen URI (TCP host:port or unix:/path). Empty: default unix socket "
    "(see evrp-server/main.cpp).");

namespace {

void resetServerListenFlags() {
  FLAGS_log_level = "info";
  FLAGS_listen = "";
}

}  // namespace

void parseServerArgvInto(ISetting& options, int argc, char* argv[]) {
  resetServerListenFlags();

  const std::string program = (argc > 0 && argv[0] && argv[0][0])
                                  ? std::string(argv[0])
                                  : std::string("evrp-server");

  std::vector<std::string> owned;
  owned.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) {
    owned.emplace_back(argv[i] ? argv[i] : "");
  }

  std::vector<char*> argv_ptrs;
  argv_ptrs.reserve(owned.size() + 1);
  for (auto& s : owned) {
    argv_ptrs.push_back(s.data());
  }
  argv_ptrs.push_back(nullptr);

  int argcMut = static_cast<int>(owned.size());
  char** argvMut = argv_ptrs.data();
  google::ParseCommandLineFlags(&argcMut, &argvMut, true);

  options.insert("program", program);
  options.insert("logLevel", logLevelFromString(FLAGS_log_level));
  if (!FLAGS_listen.empty()) {
    options.insert(evrp::sdk::kDeviceServerListenAddress,
                   std::string(FLAGS_listen));
  }
}
