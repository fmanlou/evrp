#include "inputdevice.h"

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>

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

TEST(InputDevice, CtrlAThenReleaseShouldBeRecorded) {
  keyboard_filter_state state = {};
  std::vector<evdev::Event> emitted;

  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 1), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 1), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 0), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 0), &state,
                                          &emitted);

  ASSERT_EQ(emitted.size(), 4u);
  EXPECT_EQ(emitted[0].code, KEY_LEFTCTRL);
  EXPECT_EQ(emitted[1].code, KEY_A);
  EXPECT_EQ(emitted[2].code, KEY_A);
  EXPECT_EQ(emitted[3].code, KEY_LEFTCTRL);
}

TEST(InputDevice, CtrlAThenCtrlCShouldDropWholeCtrlWindow) {
  keyboard_filter_state state = {};
  std::vector<evdev::Event> emitted;

  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 1), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 1), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 0), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 1), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 0), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 0), &state,
                                          &emitted);

  EXPECT_TRUE(emitted.empty());
}

TEST(InputDevice, EventAfterCtrlReleaseShouldRecordNormally) {
  keyboard_filter_state state = {};
  std::vector<evdev::Event> emitted;

  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 1), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 1), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 0), &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 0), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_B, 1), &state, &emitted);

  ASSERT_EQ(emitted.size(), 1u);
  EXPECT_EQ(emitted[0].code, KEY_B);
}

