#include "argparser.h"

#include "evrp/sdk/setting/memorysetting.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/device/impl/client/devicediscovery.h"
#include "evrp/sdk/logger.h"

namespace api = evrp::device::api;

static std::vector<char *> buildArgv(std::vector<std::string> *storage) {
  std::vector<char *> argv;
  for (size_t i = 0; i < storage->size(); ++i) {
    argv.push_back(const_cast<char *>((*storage)[i].c_str()));
  }
  return argv;
}

TEST(ArgParser, UseUdpDeviceDiscoveryWhenDeviceUnset) {
  EXPECT_TRUE(evrp::device::api::useUdpDeviceDiscovery(""));
  EXPECT_FALSE(evrp::device::api::useUdpDeviceDiscovery("auto"));
  EXPECT_FALSE(evrp::device::api::useUdpDeviceDiscovery("127.0.0.1:50051"));
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

  MemorySetting map;
  parseArgvInto(map, static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(map.get<bool>("recording", false));
  EXPECT_FALSE(map.get<bool>("playback", false));
  EXPECT_TRUE(
      map.get("kinds", std::vector<api::DeviceKind>{}).empty());
}

TEST(ArgParser, ParseOptionsEnableRecordingAndKinds) {
  std::vector<std::string> storage = {"evrp", "-r", "mouse", "keyboard"};
  std::vector<char *> argv = buildArgv(&storage);

  MemorySetting map;
  parseArgvInto(map, static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(map.get<bool>("recording", false));
  auto kinds = map.get("kinds", std::vector<api::DeviceKind>{});
  ASSERT_EQ(kinds.size(), 2u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kMouse);
  EXPECT_EQ(kinds[1], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseOptionsReadsOutputPath) {
  std::vector<std::string> storage = {"evrp", "-r", "-o", "events.log",
                                      "touchpad"};
  std::vector<char *> argv = buildArgv(&storage);

  MemorySetting map;
  parseArgvInto(map, static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(map.get<bool>("recording", false));
  EXPECT_FALSE(map.get<bool>("playback", false));
  EXPECT_EQ(map.get<std::string>("outputPath", {}), "events.log");
  auto kinds = map.get("kinds", std::vector<api::DeviceKind>{});
  ASSERT_EQ(kinds.size(), 1u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kTouchpad);
}

TEST(ArgParser, ParseOptionsEnablePlaybackAndPath) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log"};
  std::vector<char *> argv = buildArgv(&storage);

  MemorySetting map;
  parseArgvInto(map, static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(map.get<bool>("recording", false));
  EXPECT_TRUE(map.get<bool>("playback", false));
  EXPECT_EQ(map.get<std::string>("playbackPath", {}), "events.log");
  EXPECT_TRUE(
      map.get("kinds", std::vector<api::DeviceKind>{}).empty());
}

TEST(ArgParser, ParseOptionsRecordDefaultsKindsWhenNoTypes) {
  std::vector<std::string> storage = {"evrp", "-r"};
  std::vector<char *> argv = buildArgv(&storage);

  MemorySetting map;
  parseArgvInto(map, static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(map.get<bool>("recording", false));
  auto kinds = map.get("kinds", std::vector<api::DeviceKind>{});
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
  MemorySetting map0;
  parseArgvInto(map0, static_cast<int>(argv0.size()), argv0.data());
  EXPECT_EQ(map0.get("logLevel", logging::LogLevel::Info),
            logging::LogLevel::Debug);

  std::vector<std::string> storage1 = {"evrp", "-r", "--log-level=debug",
                                       "keyboard"};
  std::vector<char *> argv1 = buildArgv(&storage1);
  MemorySetting map1;
  parseArgvInto(map1, static_cast<int>(argv1.size()), argv1.data());
  EXPECT_TRUE(map1.get<bool>("recording", false));
  EXPECT_EQ(map1.get("logLevel", logging::LogLevel::Info),
            logging::LogLevel::Debug);
  auto kinds = map1.get("kinds", std::vector<api::DeviceKind>{});
  ASSERT_EQ(kinds.size(), 1u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kKeyboard);
}

TEST(ArgParser, ParseOptionsPlaybackWithLogLevel) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log",
                                      "--log-level=error"};
  std::vector<char *> argv = buildArgv(&storage);

  MemorySetting map;
  parseArgvInto(map, static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(map.get<bool>("playback", false));
  EXPECT_EQ(map.get("logLevel", logging::LogLevel::Info),
            logging::LogLevel::Error);
  EXPECT_EQ(map.get<std::string>("playbackPath", {}), "events.log");
}

TEST(ArgParser, ParseOptionsDeviceOverride) {
  std::vector<std::string> storage = {"evrp", "-r", "--device=10.0.0.5:9999",
                                      "mouse"};
  std::vector<char *> argv = buildArgv(&storage);

  MemorySetting map;
  parseArgvInto(map, static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(map.get<bool>("recording", false));
  EXPECT_EQ(map.get<std::string>("device", {}), "10.0.0.5:9999");
  auto kinds = map.get("kinds", std::vector<api::DeviceKind>{});
  ASSERT_EQ(kinds.size(), 1u);
  EXPECT_EQ(kinds[0], api::DeviceKind::kMouse);
}
