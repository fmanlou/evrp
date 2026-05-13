#include "evrp/client/impl/runner.h"

#include "evrp/client/api/evrp.h"
#include "evrp/client/impl/argparser.h"
#include "evrp/sdk/logger.h"

Runner::Runner(MemorySetting settings) : settings_(std::move(settings)) {
  prog_ = settings_.get<std::string>("program", "evrp");
  recording_ = settings_.get<bool>("recording", false);
  playback_ = settings_.get<bool>("playback", false);
  playbackPath_ = settings_.get<std::string>("playbackPath", {});
  logLevel_ = settings_.get("logLevel", logging::LogLevel::Info);
}

int Runner::run() {
  logService->setLevel(logLevel_);

  int modeCount = (recording_ ? 1 : 0) + (playback_ ? 1 : 0);
  if (modeCount > 1) {
    logError("Cannot use --record and --playback at the same time.");
    printUsage(prog_.c_str());
    return 1;
  }

  if (modeCount == 0) {
    printUsage(prog_.c_str());
    return 1;
  }

  if (playback_) {
    if (playbackPath_.empty()) {
      logError("Playback (--playback) requires a file path.");
      printUsage(prog_.c_str());
      return 1;
    }
    return evrp::client::runReplay(std::move(settings_));
  }

  return evrp::client::runRecord(std::move(settings_));
}
