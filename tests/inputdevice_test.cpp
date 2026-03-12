#include "inputdevice.h"

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>

#include "keyboard/keyboarddevice.h"
#include "touchdevice.h"

static Capabilities make_base_caps(const std::string &name) {
  Capabilities caps = {};
  caps.name = name;
  return caps;
}

TEST(InputDevice, DetectTouchpadFromCapabilities) {
  Capabilities caps = make_base_caps("Synaptics TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_TRUE(is_touchpad_from_capabilities(caps));
}

TEST(InputDevice, RejectTouchpadWithoutFingerTool) {
  Capabilities caps = make_base_caps("TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  EXPECT_FALSE(is_touchpad_from_capabilities(caps));
}

TEST(InputDevice, DetectTouchscreenFromCapabilities) {
  Capabilities caps = make_base_caps("Goodix Touchscreen");
  caps.ev_abs = true;
  caps.abs_mt_position_x = true;
  EXPECT_TRUE(is_touchscreen_from_capabilities(caps));
}

TEST(InputDevice, RejectTouchscreenWhenTouchpad) {
  Capabilities caps = make_base_caps("Synaptics TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_FALSE(is_touchscreen_from_capabilities(caps));
}

TEST(InputDevice, DetectMouseFromCapabilities) {
  Capabilities caps = make_base_caps("USB Optical Mouse");
  caps.ev_rel = true;
  caps.rel_x = true;
  caps.rel_y = true;
  caps.btn_left = true;
  EXPECT_TRUE(is_mouse_from_capabilities(caps));
}

TEST(InputDevice, RejectMouseWithoutButtons) {
  Capabilities caps = make_base_caps("USB Optical Mouse");
  caps.ev_rel = true;
  caps.rel_x = true;
  caps.rel_y = true;
  EXPECT_FALSE(is_mouse_from_capabilities(caps));
}

TEST(InputDevice, DetectKeyboardFromCapabilities) {
  Capabilities caps = make_base_caps("AT Translated Set 2 keyboard");
  caps.ev_key = true;
  caps.key_enter = true;
  EXPECT_TRUE(is_keyboard_from_capabilities(caps));
}

TEST(InputDevice, RejectKeyboardWithWrongName) {
  Capabilities caps = make_base_caps("generic input device");
  caps.ev_key = true;
  caps.key_enter = true;
  EXPECT_FALSE(is_keyboard_from_capabilities(caps));
}

TEST(InputDevice, NameMatchIsCaseInsensitive) {
  Capabilities caps = make_base_caps("sYnApTiCs tOuChPaD");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_TRUE(is_touchpad_from_capabilities(caps));
}

TEST(InputDevice, CtrlAThenReleaseShouldBeRecorded) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 1),
                                          &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 1), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 0), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 0),
                                          &state, &emitted);

  ASSERT_EQ(emitted.size(), 4u);
  EXPECT_EQ(emitted[0].code, KEY_LEFTCTRL);
  EXPECT_EQ(emitted[1].code, KEY_A);
  EXPECT_EQ(emitted[2].code, KEY_A);
  EXPECT_EQ(emitted[3].code, KEY_LEFTCTRL);
}

TEST(InputDevice, CtrlAThenCtrlCShouldDropWholeCtrlWindow) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 1),
                                          &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 1), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_A, 0), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 1), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 0), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 0),
                                          &state, &emitted);

  EXPECT_TRUE(emitted.empty());
}

TEST(InputDevice, EventAfterCtrlReleaseShouldRecordNormally) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 1),
                                          &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 1), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_C, 0), &state,
                                          &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_LEFTCTRL, 0),
                                          &state, &emitted);
  process_keyboard_event_with_ctrl_filter(make_key_event(KEY_B, 1), &state,
                                          &emitted);

  ASSERT_EQ(emitted.size(), 1u);
  EXPECT_EQ(emitted[0].code, KEY_B);
}

TEST(InputDevice, TouchSegmentBreakAfterTimestamp) {
  touch_segment_state state = {};
  touch_segment_decision decision;

  decision =
      process_touch_event_for_segment(make_event(EV_KEY, BTN_TOUCH, 1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      process_touch_event_for_segment(make_event(EV_KEY, BTN_TOUCH, 0), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = process_touch_event_for_segment(
      make_event(EV_MSC, MSC_TIMESTAMP, 1234), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_TRUE(decision.emit_break_after_event);
}

TEST(InputDevice, TouchSegmentBreakBeforeNextEventWithoutTimestamp) {
  touch_segment_state state = {};
  touch_segment_decision decision;

  decision =
      process_touch_event_for_segment(make_event(EV_KEY, BTN_TOUCH, 1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      process_touch_event_for_segment(make_event(EV_KEY, BTN_TOUCH, 0), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      process_touch_event_for_segment(make_event(EV_ABS, ABS_X, 10), &state);
  EXPECT_TRUE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
}

TEST(InputDevice, MultiTouchSegmentEndsAfterAllTrackingReleased) {
  touch_segment_state state = {};
  touch_segment_decision decision;

  decision = process_touch_event_for_segment(make_event(EV_ABS, ABS_MT_SLOT, 0),
                                             &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
  decision = process_touch_event_for_segment(
      make_event(EV_ABS, ABS_MT_TRACKING_ID, 100), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = process_touch_event_for_segment(make_event(EV_ABS, ABS_MT_SLOT, 1),
                                             &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
  decision = process_touch_event_for_segment(
      make_event(EV_ABS, ABS_MT_TRACKING_ID, 200), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = process_touch_event_for_segment(
      make_event(EV_ABS, ABS_MT_TRACKING_ID, -1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = process_touch_event_for_segment(make_event(EV_ABS, ABS_MT_SLOT, 0),
                                             &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
  decision = process_touch_event_for_segment(
      make_event(EV_ABS, ABS_MT_TRACKING_ID, -1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      process_touch_event_for_segment(make_event(EV_REL, REL_X, 1), &state);
  EXPECT_TRUE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
}
