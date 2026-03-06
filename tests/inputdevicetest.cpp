#include "inputdevice.h"

#include <gtest/gtest.h>

static evdev::Capabilities make_base_caps(const std::string& name) {
  evdev::Capabilities caps = {};
  caps.name = name;
  return caps;
}

TEST(InputDevice, DetectTouchpadFromCapabilities) {
  evdev::Capabilities caps = make_base_caps("Synaptics TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_TRUE(is_touchpad_from_capabilities(caps));
}

TEST(InputDevice, RejectTouchpadWithoutFingerTool) {
  evdev::Capabilities caps = make_base_caps("TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  EXPECT_FALSE(is_touchpad_from_capabilities(caps));
}

TEST(InputDevice, DetectMouseFromCapabilities) {
  evdev::Capabilities caps = make_base_caps("USB Optical Mouse");
  caps.ev_rel = true;
  caps.rel_x = true;
  caps.rel_y = true;
  caps.btn_left = true;
  EXPECT_TRUE(is_mouse_from_capabilities(caps));
}

TEST(InputDevice, RejectMouseWithoutButtons) {
  evdev::Capabilities caps = make_base_caps("USB Optical Mouse");
  caps.ev_rel = true;
  caps.rel_x = true;
  caps.rel_y = true;
  EXPECT_FALSE(is_mouse_from_capabilities(caps));
}

TEST(InputDevice, DetectKeyboardFromCapabilities) {
  evdev::Capabilities caps = make_base_caps("AT Translated Set 2 keyboard");
  caps.ev_key = true;
  caps.key_enter = true;
  EXPECT_TRUE(is_keyboard_from_capabilities(caps));
}

TEST(InputDevice, RejectKeyboardWithWrongName) {
  evdev::Capabilities caps = make_base_caps("generic input device");
  caps.ev_key = true;
  caps.key_enter = true;
  EXPECT_FALSE(is_keyboard_from_capabilities(caps));
}

TEST(InputDevice, NameMatchIsCaseInsensitive) {
  evdev::Capabilities caps = make_base_caps("sYnApTiCs tOuChPaD");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_TRUE(is_touchpad_from_capabilities(caps));
}

