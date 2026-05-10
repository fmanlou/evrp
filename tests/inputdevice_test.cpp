#include "evrp/sdk/inputdevice.h"

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>

#include <vector>

#include "evrp/sdk/keyboard/keyboarddevice.h"
#include "evrp/sdk/touchdevice.h"

static Capabilities makeBaseCaps(const std::string &name) {
  Capabilities caps = {};
  caps.name = name;
  return caps;
}

TEST(InputDevice, DetectTouchpadFromCapabilities) {
  Capabilities caps = makeBaseCaps("Synaptics TouchPad");
  caps.evAbs = true;
  caps.absX = true;
  caps.btnToolFinger = true;
  EXPECT_TRUE(isTouchpadFromCapabilities(caps));
}

TEST(InputDevice, RejectTouchpadWithoutFingerTool) {
  Capabilities caps = makeBaseCaps("TouchPad");
  caps.evAbs = true;
  caps.absX = true;
  EXPECT_FALSE(isTouchpadFromCapabilities(caps));
}

TEST(InputDevice, DetectTouchscreenFromCapabilities) {
  Capabilities caps = makeBaseCaps("Goodix Touchscreen");
  caps.evAbs = true;
  caps.absMtPositionX = true;
  EXPECT_TRUE(isTouchscreenFromCapabilities(caps));
}

TEST(InputDevice, RejectTouchscreenWhenTouchpad) {
  Capabilities caps = makeBaseCaps("Synaptics TouchPad");
  caps.evAbs = true;
  caps.absX = true;
  caps.btnToolFinger = true;
  EXPECT_FALSE(isTouchscreenFromCapabilities(caps));
}

TEST(InputDevice, FindAllUnspecifiedYieldsEmpty) {
  const std::vector<std::string> v =
      findAllDevicePaths(evrp::device::api::DeviceKind::kUnspecified);
  EXPECT_TRUE(v.empty());
}

TEST(InputDevice, DetectMouseFromCapabilities) {
  Capabilities caps = makeBaseCaps("USB Optical Mouse");
  caps.evRel = true;
  caps.relX = true;
  caps.relY = true;
  caps.btnLeft = true;
  EXPECT_TRUE(isMouseFromCapabilities(caps));
}

TEST(InputDevice, RejectMouseWithoutButtons) {
  Capabilities caps = makeBaseCaps("USB Optical Mouse");
  caps.evRel = true;
  caps.relX = true;
  caps.relY = true;
  EXPECT_FALSE(isMouseFromCapabilities(caps));
}

TEST(InputDevice, DetectKeyboardFromCapabilities) {
  Capabilities caps = makeBaseCaps("AT Translated Set 2 keyboard");
  caps.evKey = true;
  caps.keyEnter = true;
  EXPECT_TRUE(isKeyboardFromCapabilities(caps));
}

TEST(InputDevice, RejectKeyboardWithWrongName) {
  Capabilities caps = makeBaseCaps("generic input device");
  caps.evKey = true;
  caps.keyEnter = true;
  EXPECT_FALSE(isKeyboardFromCapabilities(caps));
}

TEST(InputDevice, NameMatchIsCaseInsensitive) {
  Capabilities caps = makeBaseCaps("sYnApTiCs tOuChPaD");
  caps.evAbs = true;
  caps.absX = true;
  caps.btnToolFinger = true;
  EXPECT_TRUE(isTouchpadFromCapabilities(caps));
}

TEST(InputDevice, CtrlAThenReleaseShouldBeRecorded) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 1),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 0),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);

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
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 1),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 0),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 1),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 0),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);

  EXPECT_TRUE(emitted.empty());
}

TEST(InputDevice, EventAfterCtrlReleaseShouldRecordNormally) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 1),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 0),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_B, 1),
                                     KeyboardCtrlCFilterMode::kFull, &state,
                                     &emitted);

  ASSERT_EQ(emitted.size(), 1u);
  EXPECT_EQ(emitted[0].code, KEY_B);
}

TEST(InputDevice, CtrlCEndingOnlyDropsReleasePhase) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 1),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 0),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);

  ASSERT_EQ(emitted.size(), 2u);
  EXPECT_EQ(emitted[0].code, KEY_LEFTCTRL);
  EXPECT_EQ(emitted[0].value, 1);
  EXPECT_EQ(emitted[1].code, KEY_C);
  EXPECT_EQ(emitted[1].value, 1);
}

TEST(InputDevice, CtrlCEndingOnlyAfterCtrlAThenRecordsB) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 1),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_A, 0),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 1),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 0),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_B, 1),
                                     KeyboardCtrlCFilterMode::kEndingOnly,
                                     &state, &emitted);

  ASSERT_EQ(emitted.size(), 5u);
  EXPECT_EQ(emitted[0].code, KEY_LEFTCTRL);
  EXPECT_EQ(emitted[1].code, KEY_A);
  EXPECT_EQ(emitted[2].code, KEY_A);
  EXPECT_EQ(emitted[3].code, KEY_C);
  EXPECT_EQ(emitted[3].value, 1);
  EXPECT_EQ(emitted[4].code, KEY_B);
}

TEST(InputDevice, CtrlCOffPassesThroughFullChord) {
  keyboard_filter_state state = {};
  std::vector<Event> emitted;

  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 1),
                                     KeyboardCtrlCFilterMode::kOff, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 1),
                                     KeyboardCtrlCFilterMode::kOff, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_C, 0),
                                     KeyboardCtrlCFilterMode::kOff, &state,
                                     &emitted);
  processKeyboardEventWithCtrlFilter(makeKeyEvent(KEY_LEFTCTRL, 0),
                                     KeyboardCtrlCFilterMode::kOff, &state,
                                     &emitted);

  ASSERT_EQ(emitted.size(), 4u);
}

TEST(InputDevice, KeyboardCtrlCFilterModeFromLabel) {
  EXPECT_EQ(keyboardCtrlCFilterModeFromLabel(""),
            KeyboardCtrlCFilterMode::kOff);
  EXPECT_EQ(keyboardCtrlCFilterModeFromLabel("off"),
            KeyboardCtrlCFilterMode::kOff);
  EXPECT_EQ(keyboardCtrlCFilterModeFromLabel("FULL"),
            KeyboardCtrlCFilterMode::kFull);
  EXPECT_EQ(keyboardCtrlCFilterModeFromLabel(" ending "),
            KeyboardCtrlCFilterMode::kEndingOnly);
  EXPECT_EQ(keyboardCtrlCFilterModeFromLabel("endingonly"),
            KeyboardCtrlCFilterMode::kEndingOnly);
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
