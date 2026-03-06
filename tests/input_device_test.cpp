#include "input_device.h"

#include <gtest/gtest.h>

// Non-existent device paths should not be detected as any device type.
TEST(InputDevice, NonExistentPathNotTouchpad) {
  EXPECT_FALSE(is_touchpad("/dev/input/event999"));
  EXPECT_FALSE(is_touchpad("/nonexistent"));
}

TEST(InputDevice, NonExistentPathNotMouse) {
  EXPECT_FALSE(is_mouse("/dev/input/event999"));
  EXPECT_FALSE(is_mouse("/nonexistent"));
}

TEST(InputDevice, NonExistentPathNotKeyboard) {
  EXPECT_FALSE(is_keyboard("/dev/input/event999"));
  EXPECT_FALSE(is_keyboard("/nonexistent"));
}

// find_first_* return empty string or a path starting with /dev/input/event
TEST(InputDevice, FindFirstReturnsValidPathFormat) {
  std::string tp = find_first_touchpad();
  std::string m = find_first_mouse();
  std::string k = find_first_keyboard();

  auto valid_or_empty = [](const std::string& s) {
    return s.empty() || (s.find("/dev/input/event") == 0);
  };
  EXPECT_TRUE(valid_or_empty(tp));
  EXPECT_TRUE(valid_or_empty(m));
  EXPECT_TRUE(valid_or_empty(k));
}
