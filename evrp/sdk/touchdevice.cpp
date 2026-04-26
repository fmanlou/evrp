#include "evrp/sdk/touchdevice.h"

#include <linux/input-event-codes.h>

#include <cctype>
#include <string>

static bool nameLikeTouchpad(const std::string &name) {
  std::string n = name;
  for (auto &c : n) c = static_cast<char>(std::tolower(c));
  return n.find("touchpad") != std::string::npos ||
         n.find("trackpad") != std::string::npos ||
         n.find("synaptics") != std::string::npos ||
         n.find("elan") != std::string::npos;
}

bool isTouchpadFromCapabilities(const Capabilities &caps) {
  bool has_abs = caps.evAbs && (caps.absX || caps.absMtPositionX);
  bool has_finger_tool = caps.btnToolFinger || caps.btnToolDoubletap ||
                         caps.btnToolTripletap;

  return has_abs && has_finger_tool && nameLikeTouchpad(caps.name);
}

static bool nameLikeTouchscreen(const std::string &name) {
  std::string n = name;
  for (auto &c : n) c = static_cast<char>(std::tolower(c));
  return n.find("touchscreen") != std::string::npos;
}

bool isTouchscreenFromCapabilities(const Capabilities &caps) {
  if (isTouchpadFromCapabilities(caps)) return false;
  bool has_abs = caps.evAbs && (caps.absX || caps.absMtPositionX);
  return has_abs && nameLikeTouchscreen(caps.name);
}

static bool isTouchingNow(const touch_segment_state &state) {
  bool any_tool_active =
      state.tool_finger_active || state.tool_doubletap_active ||
      state.tool_tripletap_active || state.tool_quadtap_active;
  return state.active_mt_count > 0 || state.btn_touch_active || any_tool_active;
}

static bool updateTouchSegmentState(const Event &ev,
                                       touch_segment_state *state) {
  if (!state) return false;

  bool was_touching = isTouchingNow(*state);

  if (ev.type == EV_ABS) {
    if (ev.code == ABS_MT_SLOT) {
      state->current_slot = ev.value;
      if (state->current_slot >= 0 &&
          static_cast<size_t>(state->current_slot) >=
              state->slot_active.size()) {
        state->slot_active.resize(static_cast<size_t>(state->current_slot + 1),
                                  0);
      }
    } else if (ev.code == ABS_MT_TRACKING_ID && state->current_slot >= 0) {
      if (static_cast<size_t>(state->current_slot) >=
          state->slot_active.size()) {
        state->slot_active.resize(static_cast<size_t>(state->current_slot + 1),
                                  0);
      }
      char &current_active =
          state->slot_active[static_cast<size_t>(state->current_slot)];
      if (ev.value == -1) {
        if (current_active) {
          current_active = 0;
          state->active_mt_count -= 1;
        }
      } else {
        if (!current_active) {
          current_active = 1;
          state->active_mt_count += 1;
        }
      }
    }
  } else if (ev.type == EV_KEY) {
    if (ev.code == BTN_TOUCH) state->btn_touch_active = (ev.value != 0);
    if (ev.code == BTN_TOOL_FINGER) state->tool_finger_active = (ev.value != 0);
    if (ev.code == BTN_TOOL_DOUBLETAP)
      state->tool_doubletap_active = (ev.value != 0);
    if (ev.code == BTN_TOOL_TRIPLETAP)
      state->tool_tripletap_active = (ev.value != 0);
#ifdef BTN_TOOL_QUADTAP
    if (ev.code == BTN_TOOL_QUADTAP)
      state->tool_quadtap_active = (ev.value != 0);
#endif
  }

  bool is_touching = isTouchingNow(*state);
  return was_touching && !is_touching;
}

touch_segment_decision processTouchEventForSegment(
    const Event &ev, touch_segment_state *state) {
  touch_segment_decision decision = {false, false};
  if (!state) return decision;

  bool is_msc_timestamp = (ev.type == EV_MSC && ev.code == MSC_TIMESTAMP);

  if (state->pending_segment_break && !is_msc_timestamp) {
    decision.emit_break_before_event = true;
    state->pending_segment_break = false;
  }

  bool segment_ended = updateTouchSegmentState(ev, state);
  if (segment_ended) {
    state->pending_segment_break = true;
  }

  if (state->pending_segment_break && is_msc_timestamp) {
    decision.emit_break_after_event = true;
    state->pending_segment_break = false;
  }

  return decision;
}
