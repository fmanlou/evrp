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

namespace {

void resetArgFlags() {
  FLAGS_record = false;
  FLAGS_playback = "";
  FLAGS_output = "";
  FLAGS_log_level = "info";
  FLAGS_wait_leading = true;
  FLAGS_wait_trailing = true;
}

// Map legacy flags (-r/-p/-o, --log-level, --wait-*) to gflags names.
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

} // namespace

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
      << "  --help: show gflags help.\n";
}

bool parseKind(const std::string &s, evrp::device::api::DeviceKind *out_kind) {
  evrp::device::api::DeviceKind k = evrp::device::api::deviceKindFromLabel(s);
  if (k != evrp::device::api::DeviceKind::kUnspecified) {
    *out_kind = k;
    return true;
  }
  return false;
}

run_options parseOptions(int argc, char *argv[]) {
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

  // argc may shrink after merging -p/-o with their values.
  int argc_mut = static_cast<int>(owned.size());
  char **argv_mut = argv_ptrs.data();
  google::ParseCommandLineFlags(&argc_mut, &argv_mut, true);

  run_options options;
  options.recording = FLAGS_record;
  options.playback = !FLAGS_playback.empty();
  options.playback_path = FLAGS_playback;
  options.output_path = FLAGS_output;
  options.log_level = logLevelFromString(FLAGS_log_level);
  options.execute_wait_before_first = FLAGS_wait_leading;
  options.execute_wait_after_last = FLAGS_wait_trailing;

  for (int i = 1; i < argc_mut; ++i) {
    if (!argv_mut[i]) {
      continue;
    }
    evrp::device::api::DeviceKind k;
    if (parseKind(argv_mut[i], &k)) {
      options.kinds.push_back(k);
    }
  }

  if (options.recording && options.kinds.empty()) {
    options.kinds = {evrp::device::api::DeviceKind::kTouchpad,
                       evrp::device::api::DeviceKind::kTouchscreen,
                       evrp::device::api::DeviceKind::kMouse,
                       evrp::device::api::DeviceKind::kKeyboard};
  }
  return options;
}
