#include "argparser.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/logger.h"

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

  auto parsed = parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(parsed_options::boolOr(parsed, "recording"));
  EXPECT_FALSE(parsed_options::boolOr(parsed, "playback"));
  EXPECT_TRUE(parsed_options::kindsOr(parsed, "kinds").empty());
}

TEST(ArgParser, ParseOptionsEnableRecordingAndKinds) {
  std::vector<std::string> storage = {"evrp", "-r", "mouse", "keyboard"};
  std::vector<char *> argv = buildArgv(&storage);

  auto parsed = parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(parsed_options::boolOr(parsed, "recording"));
  auto kinds = parsed_options::kindsOr(parsed, "kinds");
  ASSERT_EQ(kinds.size(), 2u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kMouse);
  EXPECT_EQ(kinds[1], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseOptionsReadsOutputPath) {
  std::vector<std::string> storage = {"evrp", "-r", "-o", "events.log",
                                      "touchpad"};
  std::vector<char *> argv = buildArgv(&storage);

  auto parsed = parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(parsed_options::boolOr(parsed, "recording"));
  EXPECT_FALSE(parsed_options::boolOr(parsed, "playback"));
  EXPECT_EQ(parsed_options::stringOr(parsed, "outputPath"), "events.log");
  auto kinds = parsed_options::kindsOr(parsed, "kinds");
  ASSERT_EQ(kinds.size(), 1u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kTouchpad);
}

TEST(ArgParser, ParseOptionsEnablePlaybackAndPath) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log"};
  std::vector<char *> argv = buildArgv(&storage);

  auto parsed = parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(parsed_options::boolOr(parsed, "recording"));
  EXPECT_TRUE(parsed_options::boolOr(parsed, "playback"));
  EXPECT_EQ(parsed_options::stringOr(parsed, "playbackPath"), "events.log");
  EXPECT_TRUE(parsed_options::kindsOr(parsed, "kinds").empty());
}

TEST(ArgParser, ParseOptionsRecordDefaultsKindsWhenNoTypes) {
  std::vector<std::string> storage = {"evrp", "-r"};
  std::vector<char *> argv = buildArgv(&storage);

  auto parsed = parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(parsed_options::boolOr(parsed, "recording"));
  auto kinds = parsed_options::kindsOr(parsed, "kinds");
  ASSERT_EQ(kinds.size(), 4u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kTouchpad);
  EXPECT_EQ(kinds[1], api::DeviceKind::kTouchscreen);
  EXPECT_EQ(kinds[2], api::DeviceKind::kMouse);
  EXPECT_EQ(kinds[3], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, LogLevelFromString) {
  EXPECT_EQ(logLevelFromString("error"), logging::LogLevel::Error);
  EXPECT_EQ(logLevelFromString("warn"), logging::LogLevel::Warning);
  EXPECT_EQ(logLevelFromString("info"), logging::LogLevel::Info);
  EXPECT_EQ(logLevelFromString("debug"), logging::LogLevel::Debug);
  EXPECT_EQ(logLevelFromString("trace"), logging::LogLevel::Trace);
}

TEST(ArgParser, ParseOptionsLogLevel) {
  std::vector<std::string> storage0 = {"evrp", "--log-level=debug"};
  std::vector<char *> argv0 = buildArgv(&storage0);
  auto opt0 = parseOptions(static_cast<int>(argv0.size()), argv0.data());
  EXPECT_EQ(parsed_options::logLevelOr(opt0, "logLevel"), logging::LogLevel::Debug);

  std::vector<std::string> storage1 = {"evrp", "-r", "--log-level=debug",
                                       "keyboard"};
  std::vector<char *> argv1 = buildArgv(&storage1);
  auto opt1 = parseOptions(static_cast<int>(argv1.size()), argv1.data());
  EXPECT_TRUE(parsed_options::boolOr(opt1, "recording"));
  EXPECT_EQ(parsed_options::logLevelOr(opt1, "logLevel"), logging::LogLevel::Debug);
  auto kinds = parsed_options::kindsOr(opt1, "kinds");
  ASSERT_EQ(kinds.size(), 1u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseOptionsPlaybackWithLogLevel) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log",
                                      "--log-level=error"};
  std::vector<char *> argv = buildArgv(&storage);

  auto parsed = parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(parsed_options::boolOr(parsed, "playback"));
  EXPECT_EQ(parsed_options::logLevelOr(parsed, "logLevel"), logging::LogLevel::Error);
  EXPECT_EQ(parsed_options::stringOr(parsed, "playbackPath"), "events.log");
}

TEST(ArgParser, ParseOptionsDeviceOverride) {
  std::vector<std::string> storage = {"evrp", "-r", "--device=10.0.0.5:9999",
                                      "mouse"};
  std::vector<char *> argv = buildArgv(&storage);

  auto parsed = parseOptions(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(parsed_options::boolOr(parsed, "recording"));
  EXPECT_EQ(parsed_options::stringOr(parsed, "device"), "10.0.0.5:9999");
  auto kinds = parsed_options::kindsOr(parsed, "kinds");
  ASSERT_EQ(kinds.size(), 1u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kMouse);
}
