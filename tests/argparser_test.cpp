#include "argparser.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

static std::vector<char*> build_argv(std::vector<std::string>* storage) {
  std::vector<char*> argv;
  for (size_t i = 0; i < storage->size(); ++i) {
    argv.push_back(const_cast<char*>((*storage)[i].c_str()));
  }
  return argv;
}

TEST(ArgParser, ParseKindAcceptsKnownKinds) {
  std::string label;
  EXPECT_TRUE(parse_kind("touchpad", &label));
  EXPECT_EQ(label, "touchpad");
  EXPECT_TRUE(parse_kind("mouse", &label));
  EXPECT_EQ(label, "mouse");
  EXPECT_TRUE(parse_kind("keyboard", &label));
  EXPECT_EQ(label, "keyboard");
}

TEST(ArgParser, ParseKindRejectsUnknownKind) {
  std::string label = "init";
  EXPECT_FALSE(parse_kind("joystick", &label));
  EXPECT_EQ(label, "init");
}

TEST(ArgParser, ParseOptionsWithNoArgsDisablesRecording) {
  std::vector<std::string> storage = {"evrp"};
  std::vector<char*> argv = build_argv(&storage);

  run_options options = parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(options.recording);
  EXPECT_FALSE(options.playback);
  EXPECT_TRUE(options.kinds.empty());
}

TEST(ArgParser, ParseOptionsEnableRecordingAndKinds) {
  std::vector<std::string> storage = {"evrp", "-r", "mouse", "keyboard"};
  std::vector<char*> argv = build_argv(&storage);

  run_options options = parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  ASSERT_EQ(options.kinds.size(), 2u);
  EXPECT_EQ(options.kinds[0], "mouse");
  EXPECT_EQ(options.kinds[1], "keyboard");
}

TEST(ArgParser, ParseOptionsReadsOutputPath) {
  std::vector<std::string> storage = {"evrp", "-r", "-o", "events.log", "touchpad"};
  std::vector<char*> argv = build_argv(&storage);

  run_options options = parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  EXPECT_FALSE(options.playback);
  EXPECT_EQ(options.output_path, "events.log");
  ASSERT_EQ(options.kinds.size(), 1u);
  EXPECT_EQ(options.kinds[0], "touchpad");
}

TEST(ArgParser, ParseOptionsEnablePlaybackAndPath) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log"};
  std::vector<char*> argv = build_argv(&storage);

  run_options options = parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_FALSE(options.recording);
  EXPECT_TRUE(options.playback);
  EXPECT_EQ(options.playback_path, "events.log");
  EXPECT_TRUE(options.kinds.empty());
}

TEST(ArgParser, ParseOptionsRecordDefaultsKindsWhenNoTypes) {
  std::vector<std::string> storage = {"evrp", "-r"};
  std::vector<char*> argv = build_argv(&storage);

  run_options options = parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  ASSERT_EQ(options.kinds.size(), 3u);
  EXPECT_EQ(options.kinds[0], "touchpad");
  EXPECT_EQ(options.kinds[1], "mouse");
  EXPECT_EQ(options.kinds[2], "keyboard");
}

TEST(ArgParser, ParseOptionsQuietFlag) {
  std::vector<std::string> storage = {"evrp", "-r", "-q", "keyboard"};
  std::vector<char*> argv = build_argv(&storage);

  run_options options = parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.recording);
  EXPECT_TRUE(options.quiet);
  ASSERT_EQ(options.kinds.size(), 1u);
  EXPECT_EQ(options.kinds[0], "keyboard");
}

TEST(ArgParser, ParseOptionsPlaybackWithQuiet) {
  std::vector<std::string> storage = {"evrp", "-p", "events.log", "-q"};
  std::vector<char*> argv = build_argv(&storage);

  run_options options = parse_options(static_cast<int>(argv.size()), argv.data());
  EXPECT_TRUE(options.playback);
  EXPECT_TRUE(options.quiet);
  EXPECT_EQ(options.playback_path, "events.log");
}

