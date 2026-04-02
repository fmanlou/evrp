#include "inputdevice.h"

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>

#include "keyboard/keyboarddevice.h"
#include "touchdevice.h"

static Capabilities makeBaseCaps(const std::string &name) {
  Capabilities caps = {};
  caps.name = name;
  return caps;
}

TEST(InputDevice, DetectTouchpadFromCapabilities) {
  Capabilities caps = makeBaseCaps("Synaptics TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_TRUE(isTouchpadFromCapabilities(caps));
}

TEST(InputDevice, RejectTouchpadWithoutFingerTool) {
  Capabilities caps = makeBaseCaps("TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  EXPECT_FALSE(isTouchpadFromCapabilities(caps));
}

TEST(InputDevice, DetectTouchscreenFromCapabilities) {
  Capabilities caps = makeBaseCaps("Goodix Touchscreen");
  caps.ev_abs = true;
  caps.abs_mt_position_x = true;
  EXPECT_TRUE(isTouchscreenFromCapabilities(caps));
}

TEST(InputDevice, RejectTouchscreenWhenTouchpad) {
  Capabilities caps = makeBaseCaps("Synaptics TouchPad");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_FALSE(isTouchscreenFromCapabilities(caps));
}

TEST(InputDevice, DetectMouseFromCapabilities) {
  Capabilities caps = makeBaseCaps("USB Optical Mouse");
  caps.ev_rel = true;
  caps.rel_x = true;
  caps.rel_y = true;
  caps.btn_left = true;
  EXPECT_TRUE(isMouseFromCapabilities(caps));
}

TEST(InputDevice, RejectMouseWithoutButtons) {
  Capabilities caps = makeBaseCaps("USB Optical Mouse");
  caps.ev_rel = true;
  caps.rel_x = true;
  caps.rel_y = true;
  EXPECT_FALSE(isMouseFromCapabilities(caps));
}

TEST(InputDevice, DetectKeyboardFromCapabilities) {
  Capabilities caps = makeBaseCaps("AT Translated Set 2 keyboard");
  caps.ev_key = true;
  caps.key_enter = true;
  EXPECT_TRUE(isKeyboardFromCapabilities(caps));
}

TEST(InputDevice, RejectKeyboardWithWrongName) {
  Capabilities caps = makeBaseCaps("generic input device");
  caps.ev_key = true;
  caps.key_enter = true;
  EXPECT_FALSE(isKeyboardFromCapabilities(caps));
}

TEST(InputDevice, NameMatchIsCaseInsensitive) {
  Capabilities caps = makeBaseCaps("sYnApTiCs tOuChPaD");
  caps.ev_abs = true;
  caps.abs_x = true;
  caps.btn_tool_finger = true;
  EXPECT_TRUE(isTouchpadFromCapabilities(caps));
}

TEST(InputDevice, CtrlAThenReleaseShouldBeRecorded) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                          &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 1), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 0), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
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

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                          &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 1), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 0), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 1), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 0), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                          &state, &emitted);

  EXPECT_TRUE(emitted.empty());
}

TEST(InputDevice, EventAfterCtrlReleaseShouldRecordNormally) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                          &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 1), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 0), &state,
                                          &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                          &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_B, 1), &state,
                                          &emitted);

  ASSERT_EQ(emitted.size(), 1u);
  EXPECT_EQ(emitted[0].code, KEY_B);
}

TEST(InputDevice, TouchSegmentBreakAfterTimestamp) {
  touch_segment_state state = {};
  touch_segment_decision decision;

  decision =
      processTouchEventForSegment(makeEvent(EV_KEY, BTN_TOUCH, 1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      processTouchEventForSegment(makeEvent(EV_KEY, BTN_TOUCH, 0), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = processTouchEventForSegment(
      makeEvent(EV_MSC, MSC_TIMESTAMP, 1234), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_TRUE(decision.emit_break_after_event);
}

TEST(InputDevice, TouchSegmentBreakBeforeNextEventWithoutTimestamp) {
  touch_segment_state state = {};
  touch_segment_decision decision;

  decision =
      processTouchEventForSegment(makeEvent(EV_KEY, BTN_TOUCH, 1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      processTouchEventForSegment(makeEvent(EV_KEY, BTN_TOUCH, 0), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      processTouchEventForSegment(makeEvent(EV_ABS, ABS_X, 10), &state);
  EXPECT_TRUE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
}

TEST(InputDevice, MultiTouchSegmentEndsAfterAllTrackingReleased) {
  touch_segment_state state = {};
  touch_segment_decision decision;

  decision = processTouchEventForSegment(makeEvent(EV_ABS, ABS_MT_SLOT, 0),
                                             &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
  decision = processTouchEventForSegment(
      makeEvent(EV_ABS, ABS_MT_TRACKING_ID, 100), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = processTouchEventForSegment(makeEvent(EV_ABS, ABS_MT_SLOT, 1),
                                             &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
  decision = processTouchEventForSegment(
      makeEvent(EV_ABS, ABS_MT_TRACKING_ID, 200), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = processTouchEventForSegment(
      makeEvent(EV_ABS, ABS_MT_TRACKING_ID, -1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision = processTouchEventForSegment(makeEvent(EV_ABS, ABS_MT_SLOT, 0),
                                             &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
  decision = processTouchEventForSegment(
      makeEvent(EV_ABS, ABS_MT_TRACKING_ID, -1), &state);
  EXPECT_FALSE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);

  decision =
      processTouchEventForSegment(makeEvent(EV_REL, REL_X, 1), &state);
  EXPECT_TRUE(decision.emit_break_before_event);
  EXPECT_FALSE(decision.emit_break_after_event);
}
