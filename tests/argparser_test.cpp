#include "argparser.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "deviceid.h"
#include "logger.h"

static std::vector<char *> build_argv(std::vector<std::string> *storage) {
  std::vector<char *> argv;
  for (size_t i = 0; i < storage->size(); ++i) {
    argv.push_back(const_cast<char *>((*storage)[i].c_str()));
  }
  return argv;
}

TEST(ArgParser, ParseKindAcceptsKnownKinds) {
  DeviceId id;
  EXPECT_TRUE(parse_kind("touchpad", &id));
  EXPECT_EQ(id, DeviceId::Touchpad);
  EXPECT_TRUE(parse_kind("mouse", &id));
  EXPECT_EQ(id, DeviceId::Mouse);
  EXPECT_TRUE(parse_kind("keyboard", &id));
  EXPECT_EQ(id, DeviceId::Keyboard);
}

TEST(ArgParser, ParseKindRejectsUnknownKind) {
  DeviceId id = DeviceId::Keyboard;
  EXPECT_FALSE(parse_kind("joystick", &id));
  EXPECT_EQ(id, DeviceId::Keyboard);
}

TEST(ArgParser, ParseOptionsWithNoArgsDisablesRecording) {
  std::vector<std::string> storage = {"evrp"};
  std::vector<char *> argv = build_argv(&storage);

  run_options options =
      parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(options.recording);
  EXPECT_FALSE(options.playback);
  EXPECT_TRUE(options.kinds.empty());
}

TEST(ArgParser, ParseOptionsEnableRecordingAndKinds) {
  std::vector<std::string> storage = {"evrp", "-r", "mouse", "keyboard"};
  std::vector<char *> argv = build_argv(&storage);

  run_options options =
      parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  ASSERT_EQ(options.kinds.size(), 2u);
  EXPECT_EQ(options.kinds[0], DeviceId::Mouse);
  EXPECT_EQ(options.kinds[1], DeviceId::Keyboard);
}

TEST(ArgParser, ParseOptionsReadsOutputPath) {
  std::vector<std::string> storage = {"evrp", "-r", "-o", "events.log",
                                      "touchpad"};
  std::vector<char *> argv = build_argv(&storage);

  run_options options =
      parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  EXPECT_FALSE(options.playback);
  EXPECT_EQ(options.output_path, "events.log");
  ASSERT_EQ(options.kinds.size(), 1u);
  EXPECT_EQ(options.kinds[0], DeviceId::Touchpad);
}

TEST(ArgParser, ParseOptionsEnablePlaybackAndPath) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log"};
  std::vector<char *> argv = build_argv(&storage);

  run_options options =
      parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(options.recording);
  EXPECT_TRUE(options.playback);
  EXPECT_EQ(options.playback_path, "events.log");
  EXPECT_TRUE(options.kinds.empty());
}

TEST(ArgParser, ParseOptionsRecordDefaultsKindsWhenNoTypes) {
  std::vector<std::string> storage = {"evrp", "-r"};
  std::vector<char *> argv = build_argv(&storage);

  run_options options =
      parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  ASSERT_EQ(options.kinds.size(), 3u);
  EXPECT_EQ(options.kinds[0], DeviceId::Touchpad);
  EXPECT_EQ(options.kinds[1], DeviceId::Mouse);
  EXPECT_EQ(options.kinds[2], DeviceId::Keyboard);
}

TEST(ArgParser, LogLevelFromString) {
  EXPECT_EQ(log_level_from_string("error"), LogLevel::Error);
  EXPECT_EQ(log_level_from_string("warn"), LogLevel::Warn);
  EXPECT_EQ(log_level_from_string("info"), LogLevel::Info);
  EXPECT_EQ(log_level_from_string("debug"), LogLevel::Debug);
  EXPECT_EQ(log_level_from_string("trace"), LogLevel::Trace);
}

TEST(ArgParser, ParseOptionsLogLevel) {
  // Standalone --log-level
  std::vector<std::string> storage0 = {"evrp", "--log-level=debug"};
  std::vector<char *> argv0 = build_argv(&storage0);
  run_options opt0 =
      parse_options(static_cast<int>(argv0.size()), argv0.data());
  EXPECT_EQ(opt0.log_level, LogLevel::Debug);

  // --log-level in -r block
  std::vector<std::string> storage1 = {"evrp", "-r", "--log-level=debug",
                                       "keyboard"};
  std::vector<char *> argv1 = build_argv(&storage1);
  run_options opt1 =
      parse_options(static_cast<int>(argv1.size()), argv1.data());
  EXPECT_TRUE(opt1.recording);
  EXPECT_EQ(opt1.log_level, LogLevel::Debug);
  ASSERT_EQ(opt1.kinds.size(), 1u);
  EXPECT_EQ(opt1.kinds[0], DeviceId::Keyboard);
}

TEST(ArgParser, ParseOptionsPlaybackWithLogLevel) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log",
                                      "--log-level=error"};
  std::vector<char *> argv = build_argv(&storage);

  run_options options =
      parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.playback);
  EXPECT_EQ(options.log_level, LogLevel::Error);
  EXPECT_EQ(options.playback_path, "events.log");
}
