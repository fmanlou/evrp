#include "argparser.h"

#include <gflags/gflags.h>

#include <cctype>

#include <iostream>
#include <string>
#include <vector>

#include "evrp/sdk/logger.h"
#include "evrp/device/internal/discovery/devicediscoverysettings.h"

DECLARE_int32(discovery_port);
DECLARE_string(discovery_link_mode);

DEFINE_bool(
    record, false,
    "Start recording. Output: --output|-o or -r OUTPUT_FILE. Select devices "
    "with "
    "--kind=KIND,... (touchpad|touchscreen|mouse|keyboard); omit --kind to "
    "record "
    "all four. The token after -r is always the output path, not a device "
    "kind.");
DEFINE_string(kind, "",
              "Recording: comma-separated device kinds (touchpad, touchscreen, "
              "mouse, "
              "keyboard). Empty means record all kinds when recording.");
DEFINE_string(playback, "",
              "Playback: read events or run Lua script (.lua) from this file.");
DEFINE_string(output, "",
              "Recording: write events to this file (default: stdout).");
DEFINE_string(log_level, "info", "Log level: error|warn|info|debug|trace.");
DEFINE_bool(
    wait_leading, true,
    "Playback: execute [leading] wait (see --nowait_leading to disable).");
DEFINE_bool(
    wait_trailing, true,
    "Playback: execute [trailing] wait (see --nowait_trailing to disable).");
DEFINE_string(
    device, "",
    "evrp-device gRPC host:port. If empty, use UDP discovery (--discovery_port, "
    "--discovery_link_mode). Same-machine targets are tried first.");
DEFINE_string(
    keyboard_ctrl_c_filter, "ending",
    "Recording keyboard: off (no filter), full (drop whole Ctrl+C chord), "
    "ending (drop KEY_C release and Ctrl release after Ctrl+C press). Default: "
    "ending.");

namespace {

void resetArgFlags() {
  FLAGS_record = false;
  FLAGS_playback = "";
  FLAGS_output = "";
  FLAGS_kind = "";
  FLAGS_log_level = "info";
  FLAGS_wait_leading = true;
  FLAGS_wait_trailing = true;
  FLAGS_device = "";
  FLAGS_keyboard_ctrl_c_filter = "ending";
}

void normalizeLegacyArgs(std::vector<std::string>* args) {
  auto& a = *args;
  for (size_t i = 1; i < a.size();) {
    if (a[i] == "-r") {
      if (i + 1 < a.size()) {
        const std::string& next = a[i + 1];
        if (!next.empty() && next[0] != '-') {
          a[i] = "--record";
          a[i + 1] = "--output=" + next;
          i += 2;
          continue;
        }
      }
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

}  // namespace

void printUsage(const char* prog) {
  std::cout
      << "Usage: " << prog
      << " --record|-r [--output=FILE|-o FILE|-r OUTPUT] "
         "[--kind=TYPES] [--log_level=LEVEL|--log-level=LEVEL] ...\n"
      << "       " << prog
      << " --playback=FILE|-p FILE [--log_level=LEVEL|--log-level=LEVEL]\n"
      << "  --record / -r: start recording. The token after -r (if present and "
         "not another flag) is the output file (--output equivalence). Device "
         "kinds "
         "are set only via --kind=... .\n"
      << "  --kind=comma,separated,...: recording device kinds "
         "(touchpad|touchscreen|mouse|keyboard). "
         "Omit to record all four.\n"
      << "  --playback=FILE / -p FILE: playback events or run Lua script "
         "(.lua). "
         "Non-event lines in event files are executed as Lua.\n"
      << "  --output=FILE / -o FILE: write recording to FILE (default: "
         "stdout).\n"
      << "  --log_level=LEVEL / --log-level=LEVEL: error|warn|info|debug|trace "
         "(default: info).\n"
      << "  --wait_leading / --nowait_leading (or --wait-leading=yes|no): "
         "during "
         "playback, execute [leading] wait (default: wait).\n"
      << "  --wait_trailing / --nowait_trailing (or --wait-trailing=yes|no): "
         "during "
         "playback, execute [trailing] wait (default: wait).\n"
      << "  --device=HOST:PORT: evrp-device gRPC; omit for UDP discovery "
         "(--discovery_port / --discovery_link_mode).\n"
      << "  --keyboard_ctrl_c_filter=off|full|ending: recording keyboard Ctrl+C "
         "filter (default: ending).\n"
      << "  --help: show gflags help.\n";
}

bool parseKind(const std::string& s, evrp::device::api::DeviceKind* outKind) {
  evrp::device::api::DeviceKind k = evrp::device::api::deviceKindFromLabel(s);
  if (k != evrp::device::api::DeviceKind::kUnspecified) {
    *outKind = k;
    return true;
  }
  return false;
}

static std::vector<evrp::device::api::DeviceKind> kindsFromKindFlag() {
  std::vector<evrp::device::api::DeviceKind> kinds;
  const std::string& raw = FLAGS_kind;
  if (raw.empty()) {
    return kinds;
  }
  size_t segmentStart = 0;
  while (segmentStart < raw.size()) {
    while (segmentStart < raw.size() &&
           std::isspace(static_cast<unsigned char>(raw[segmentStart]))) {
      ++segmentStart;
    }
    if (segmentStart >= raw.size()) {
      break;
    }
    const size_t comma = raw.find(',', segmentStart);
    const size_t segmentEnd =
        comma == std::string::npos ? raw.size() : comma;
    size_t left = segmentStart;
    size_t right = segmentEnd;
    while (left < right &&
           std::isspace(static_cast<unsigned char>(raw[left]))) {
      ++left;
    }
    while (right > left &&
           std::isspace(static_cast<unsigned char>(raw[right - 1]))) {
      --right;
    }
    if (left < right) {
      const std::string token = raw.substr(left, right - left);
      evrp::device::api::DeviceKind k{};
      if (parseKind(token, &k)) {
        kinds.push_back(k);
      }
    }
    if (comma == std::string::npos) {
      break;
    }
    segmentStart = comma + 1;
  }
  return kinds;
}

void parseArgvInto(ISetting& options, int argc, char* argv[]) {
  resetArgFlags();

  const std::string program = (argc > 0 && argv[0] && argv[0][0])
                                  ? std::string(argv[0])
                                  : std::string("evrp");

  std::vector<std::string> owned;
  owned.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) {
    owned.emplace_back(argv[i] ? argv[i] : "");
  }
  normalizeLegacyArgs(&owned);
  std::vector<char*> argv_ptrs;
  argv_ptrs.reserve(static_cast<size_t>(argc) + 1);
  for (auto& s : owned) {
    argv_ptrs.push_back(s.data());
  }
  argv_ptrs.push_back(nullptr);

  int argcMut = static_cast<int>(owned.size());
  char** argvMut = argv_ptrs.data();
  google::ParseCommandLineFlags(&argcMut, &argvMut, true);

  bool recording = FLAGS_record;
  bool playback = !FLAGS_playback.empty();
  std::string playbackPath = FLAGS_playback;
  std::string outputPath = FLAGS_output;
  std::string device = FLAGS_device;
  logging::LogLevel logLevel = logLevelFromString(FLAGS_log_level);
  bool executeWaitBeforeFirst = FLAGS_wait_leading;
  bool executeWaitAfterLast = FLAGS_wait_trailing;

  std::vector<evrp::device::api::DeviceKind> kinds = kindsFromKindFlag();

  if (recording && kinds.empty()) {
    kinds = {evrp::device::api::DeviceKind::kTouchpad,
             evrp::device::api::DeviceKind::kTouchscreen,
             evrp::device::api::DeviceKind::kMouse,
             evrp::device::api::DeviceKind::kKeyboard};
  }

  options.insert("program", program);
  options.insert("recording", recording);
  options.insert("playback", playback);
  options.insert("logLevel", logLevel);
  options.insert("playbackPath", std::move(playbackPath));
  options.insert("outputPath", std::move(outputPath));
  options.insert("device", std::move(device));
  options.insert("kinds", std::move(kinds));
  options.insert("executeWaitBeforeFirst", executeWaitBeforeFirst);
  options.insert("executeWaitAfterLast", executeWaitAfterLast);
  options.insert("keyboardCtrlCFilter", FLAGS_keyboard_ctrl_c_filter);
  options.insert(evrp::sdk::kDeviceDiscoverySettingPort, FLAGS_discovery_port);
  options.insert(evrp::sdk::kDeviceDiscoverySettingLinkMode,
                 FLAGS_discovery_link_mode);
}
