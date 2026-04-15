#include "argparser.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "logger.h"

namespace api = evrp::device::api;

static std::vector<char *> buildArgv(std::vector<std::string> *storage) {
  std::vector<char *> argv;
  for (size_t i = 0; i < storage->size(); ++i) {
    argv.push_back(const_cast<char *>((*storage)[i].c_str()));
  }
  return argv;
}

TEST(ArgParser, ParseKindAcceptsKnownKinds) {
  api::DeviceKind k;
  EXPECT_TRUE(parseKind("touchpad", &k));
  EXPECT_EQ(k, api::DeviceKind::kTouchpad);
  EXPECT_TRUE(parseKind("touchscreen", &k));
  EXPECT_EQ(k, api::DeviceKind::kTouchscreen);
  EXPECT_TRUE(parseKind("mouse", &k));
  EXPECT_EQ(k, api::DeviceKind::kMouse);
  EXPECT_TRUE(parseKind("keyboard", &k));
  EXPECT_EQ(k, api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseKindRejectsUnknownKind) {
  api::DeviceKind k = api::DeviceKind::kKeyboard;
  EXPECT_FALSE(parseKind("joystick", &k));
  EXPECT_EQ(k, api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseOptionsWithNoArgsDisablesRecording) {
  std::vector<std::string> storage = {"evrp"};
  std::vector<char *> argv = buildArgv(&storage);

  RunOptions options =
      parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(options.recording);
  EXPECT_FALSE(options.playback);
  EXPECT_TRUE(options.kinds.empty());
}

TEST(ArgParser, ParseOptionsEnableRecordingAndKinds) {
  std::vector<std::string> storage = {"evrp", "-r", "mouse", "keyboard"};
  std::vector<char *> argv = buildArgv(&storage);

  RunOptions options =
      parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  ASSERT_EQ(options.kinds.size(), 2u);
  EXPECT_EQ(options.kinds[0], api::DeviceKind::kMouse);
  EXPECT_EQ(options.kinds[1], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseOptionsReadsOutputPath) {
  std::vector<std::string> storage = {"evrp", "-r", "-o", "events.log",
                                      "touchpad"};
  std::vector<char *> argv = buildArgv(&storage);

  RunOptions options =
      parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  EXPECT_FALSE(options.playback);
  EXPECT_EQ(options.outputPath, "events.log");
  ASSERT_EQ(options.kinds.size(), 1u);
  EXPECT_EQ(options.kinds[0], api::DeviceKind::kTouchpad);
}

TEST(ArgParser, ParseOptionsEnablePlaybackAndPath) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log"};
  std::vector<char *> argv = buildArgv(&storage);

  RunOptions options =
      parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(options.recording);
  EXPECT_TRUE(options.playback);
  EXPECT_EQ(options.playbackPath, "events.log");
  EXPECT_TRUE(options.kinds.empty());
}

TEST(ArgParser, ParseOptionsRecordDefaultsKindsWhenNoTypes) {
  std::vector<std::string> storage = {"evrp", "-r"};
  std::vector<char *> argv = buildArgv(&storage);

  RunOptions options =
      parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  ASSERT_EQ(options.kinds.size(), 4u);
  EXPECT_EQ(options.kinds[0], api::DeviceKind::kTouchpad);
  EXPECT_EQ(options.kinds[1], api::DeviceKind::kTouchscreen);
  EXPECT_EQ(options.kinds[2], api::DeviceKind::kMouse);
  EXPECT_EQ(options.kinds[3], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, LogLevelFromString) {
  EXPECT_EQ(logLevelFromString("error"), LogLevel::Error);
  EXPECT_EQ(logLevelFromString("warn"), LogLevel::Warn);
  EXPECT_EQ(logLevelFromString("info"), LogLevel::Info);
  EXPECT_EQ(logLevelFromString("debug"), LogLevel::Debug);
  EXPECT_EQ(logLevelFromString("trace"), LogLevel::Trace);
}

TEST(ArgParser, ParseOptionsLogLevel) {
  // Standalone --log-level
  std::vector<std::string> storage0 = {"evrp", "--log-level=debug"};
  std::vector<char *> argv0 = buildArgv(&storage0);
  RunOptions opt0 =
      parseOptions(static_cast<int>(argv0.size()), argv0.data());
  EXPECT_EQ(opt0.logLevel, LogLevel::Debug);

  // --log-level in -r block
  std::vector<std::string> storage1 = {"evrp", "-r", "--log-level=debug",
                                       "keyboard"};
  std::vector<char *> argv1 = buildArgv(&storage1);
  RunOptions opt1 =
      parseOptions(static_cast<int>(argv1.size()), argv1.data());
  EXPECT_TRUE(opt1.recording);
  EXPECT_EQ(opt1.logLevel, LogLevel::Debug);
  ASSERT_EQ(opt1.kinds.size(), 1u);
  EXPECT_EQ(opt1.kinds[0], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseOptionsPlaybackWithLogLevel) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log",
                                      "--log-level=error"};
  std::vector<char *> argv = buildArgv(&storage);

  RunOptions options =
      parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.playback);
  EXPECT_EQ(options.logLevel, LogLevel::Error);
  EXPECT_EQ(options.playbackPath, "events.log");
}
