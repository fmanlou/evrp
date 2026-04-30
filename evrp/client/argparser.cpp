#include "argparser.h"

#include <gflags/gflags.h>

#include <iostream>
#include <string>
#include <vector>

DEFINE_bool(record, false,
            "Start recording. Remaining args: device kinds (touchpad, touchscreen, "
            "mouse, keyboard). If none, record all four.");
DEFINE_string(playback, "",
              "Playback: read events or run Lua script (.lua) from this file.");
DEFINE_string(output, "",
              "Recording: write events to this file (default: stdout).");
DEFINE_string(log_level, "info",
              "Log level: error|warn|info|debug|trace.");
DEFINE_bool(wait_leading, true,
            "Playback: execute [leading] wait (see --nowait_leading to disable).");
DEFINE_bool(wait_trailing, true,
            "Playback: execute [trailing] wait (see --nowait_trailing to disable).");
DEFINE_string(device, "127.0.0.1:50051",
              "evrp-device gRPC address (host:port; same default listen as evrp-device).");

namespace {

void resetArgFlags() {
  FLAGS_record = false;
  FLAGS_playback = "";
  FLAGS_output = "";
  FLAGS_log_level = "info";
  FLAGS_wait_leading = true;
  FLAGS_wait_trailing = true;
  FLAGS_device = "127.0.0.1:50051";
}

void normalizeLegacyArgs(std::vector<std::string> *args) {
  auto &a = *args;
  for (size_t i = 1; i < a.size();) {
    if (a[i] == "-r") {
      a[i] = "--record";
      ++i;
    } else if (a[i] == "-p" && i + 1 < a.size()) {
      a[i] = "--playback=" + a[i + 1];
      a.erase(a.begin() + static_cast<long>(i) + 1);
    } else if (a[i] == "-o" && i + 1 < a.size()) {
      a[i] = "--output=" + a[i + 1];
      a.erase(a.begin() + static_cast<long>(i) + 1);
    } else if (a[i].rfind("--log-level=", 0) == 0) {
      a[i] = "--log_level=" + a[i].substr(12);
      ++i;
    } else if (a[i].rfind("--wait-leading=", 0) == 0) {
      std::string v = a[i].substr(15);
      a[i] = (v == "yes" || v == "true" || v == "1") ? "--wait_leading"
                                                     : "--nowait_leading";
      ++i;
    } else if (a[i].rfind("--wait-trailing=", 0) == 0) {
      std::string v = a[i].substr(16);
      a[i] = (v == "yes" || v == "true" || v == "1") ? "--wait_trailing"
                                                     : "--nowait_trailing";
      ++i;
    } else {
      ++i;
    }
  }
}

} 

namespace parsed_options {

std::string stringOr(const std::map<std::string, std::any>& m,
                     const std::string& key, std::string defaultValue) {
  auto it = m.find(key);
  if (it == m.end()) return defaultValue;
  if (auto* p = std::any_cast<std::string>(&it->second)) return *p;
  return defaultValue;
}

bool boolOr(const std::map<std::string, std::any>& m, const std::string& key,
            bool defaultValue) {
  auto it = m.find(key);
  if (it == m.end()) return defaultValue;
  if (auto* p = std::any_cast<bool>(&it->second)) return *p;
  return defaultValue;
}

logging::LogLevel logLevelOr(const std::map<std::string, std::any>& m,
                             const std::string& key, logging::LogLevel defaultValue) {
  auto it = m.find(key);
  if (it == m.end()) return defaultValue;
  if (auto* p = std::any_cast<logging::LogLevel>(&it->second)) return *p;
  return defaultValue;
}

std::vector<evrp::device::api::DeviceKind> kindsOr(
    const std::map<std::string, std::any>& m, const std::string& key,
    std::vector<evrp::device::api::DeviceKind> defaultValue) {
  auto it = m.find(key);
  if (it == m.end()) return defaultValue;
  if (auto* p =
          std::any_cast<std::vector<evrp::device::api::DeviceKind>>(&it->second)) {
    return *p;
  }
  return defaultValue;
}

}  // namespace parsed_options

void printUsage(const char *prog) {
  std::cout
      << "Usage: " << prog
      << " --record|-r [-o FILE|--output=FILE] [--log_level=LEVEL|--log-level=LEVEL] "
         "[touchpad] [touchscreen] [mouse] [keyboard] ...\n"
      << "       " << prog
      << " --playback=FILE|-p FILE [--log_level=LEVEL|--log-level=LEVEL]\n"
      << "  --record / -r: start recording. With no device kinds, record touchpad, "
         "touchscreen, mouse, keyboard.\n"
      << "  --playback=FILE / -p FILE: playback events or run Lua script (.lua). "
         "Non-event lines in event files are executed as Lua.\n"
      << "  --output=FILE / -o FILE: write recording to FILE (default: stdout).\n"
      << "  --log_level=LEVEL / --log-level=LEVEL: error|warn|info|debug|trace "
         "(default: info).\n"
      << "  --wait_leading / --nowait_leading (or --wait-leading=yes|no): during "
         "playback, execute [leading] wait (default: wait).\n"
      << "  --wait_trailing / --nowait_trailing (or --wait-trailing=yes|no): during "
         "playback, execute [trailing] wait (default: wait).\n"
      << "  --device=HOST:PORT: evrp-device server (default: 127.0.0.1:50051).\n"
      << "  --help: show gflags help.\n";
}

bool parseKind(const std::string &s, evrp::device::api::DeviceKind *outKind) {
  evrp::device::api::DeviceKind k = evrp::device::api::deviceKindFromLabel(s);
  if (k != evrp::device::api::DeviceKind::kUnspecified) {
    *outKind = k;
    return true;
  }
  return false;
}

RunOptions parseOptions(int argc, char *argv[]) {
  resetArgFlags();

  std::vector<std::string> owned;
  owned.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) {
    owned.emplace_back(argv[i] ? argv[i] : "");
  }
  normalizeLegacyArgs(&owned);
  std::vector<char *> argv_ptrs;
  argv_ptrs.reserve(static_cast<size_t>(argc) + 1);
  for (auto &s : owned) {
    argv_ptrs.push_back(s.data());
  }
  argv_ptrs.push_back(nullptr);

  
  int argcMut = static_cast<int>(owned.size());
  char **argvMut = argv_ptrs.data();
  google::ParseCommandLineFlags(&argcMut, &argvMut, true);

  RunOptions options;
  options.recording = FLAGS_record;
  options.playback = !FLAGS_playback.empty();
  options.playbackPath = FLAGS_playback;
  options.outputPath = FLAGS_output;
  options.device = FLAGS_device;
  options.logLevel = logLevelFromString(FLAGS_log_level);
  options.executeWaitBeforeFirst = FLAGS_wait_leading;
  options.executeWaitAfterLast = FLAGS_wait_trailing;

  for (int i = 1; i < argcMut; ++i) {
    if (!argvMut[i]) {
      continue;
    }
    evrp::device::api::DeviceKind k;
    if (parseKind(argvMut[i], &k)) {
      options.kinds.push_back(k);
    }
  }

  if (options.recording && options.kinds.empty()) {
    options.kinds = {evrp::device::api::DeviceKind::kTouchpad,
                       evrp::device::api::DeviceKind::kTouchscreen,
                       evrp::device::api::DeviceKind::kMouse,
                       evrp::device::api::DeviceKind::kKeyboard};
  }

  options.parsed["recording"] = options.recording;
  options.parsed["playback"] = options.playback;
  options.parsed["logLevel"] = options.logLevel;
  options.parsed["playbackPath"] = options.playbackPath;
  options.parsed["outputPath"] = options.outputPath;
  options.parsed["device"] = options.device;
  options.parsed["kinds"] = options.kinds;
  options.parsed["executeWaitBeforeFirst"] = options.executeWaitBeforeFirst;
  options.parsed["executeWaitAfterLast"] = options.executeWaitAfterLast;

  return options;
}
