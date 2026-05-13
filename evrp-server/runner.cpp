#include "evrp-server/runner.h"

#include <memory>

#include "evrp-server/argparser.h"
#include "evrp/server/api/evrp.h"
#include "evrp/server/impl/server/localevrp.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

RunnerSetting::RunnerSetting(const ISetting& settings) {
  program = settings.get<std::string>("program", "evrp");
  recording = settings.get<bool>("recording", false);
  playback = settings.get<bool>("playback", false);
  playbackPath = settings.get<std::string>("playbackPath", {});
  logLevel = settings.get("logLevel", logging::LogLevel::Info);
}

Runner::Runner(std::shared_ptr<MemorySetting> settings)
    : settings_(std::move(settings)), runnerSetting_(*settings_) {}

int Runner::run() {
  logService->setLevel(runnerSetting_.logLevel);

  int modeCount =
      (runnerSetting_.recording ? 1 : 0) + (runnerSetting_.playback ? 1 : 0);
  if (modeCount > 1) {
    logError("Cannot use --record and --playback at the same time.");
    printUsage(runnerSetting_.program.c_str());
    return 1;
  }

  if (modeCount == 0) {
    printUsage(runnerSetting_.program.c_str());
    return 1;
  }

  evrp::server::LocalEvrp evrp;

  if (runnerSetting_.playback) {
    if (runnerSetting_.playbackPath.empty()) {
      logError("Playback (--playback) requires a file path.");
      printUsage(runnerSetting_.program.c_str());
      return 1;
    }
    return evrp.replay(settings_);
  }

  return evrp.record(settings_);
}
