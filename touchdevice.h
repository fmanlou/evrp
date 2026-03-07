#pragma once

#include "evdev.h"

#include <vector>

struct touch_segment_state {
  int current_slot;
  std::vector<char> slot_active;
  int active_mt_count;
  bool btn_touch_active;
  bool tool_finger_active;
  bool tool_doubletap_active;
  bool tool_tripletap_active;
  bool tool_quadtap_active;
  bool pending_segment_break;
};

struct touch_segment_decision {
  bool emit_break_before_event;
  bool emit_break_after_event;
};

touch_segment_decision process_touch_event_for_segment(
    const Event& ev, touch_segment_state* state);
bool is_touchpad_from_capabilities(const Capabilities& caps);

