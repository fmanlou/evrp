#include "evrp/sdk/eventcomposer.h"

#include <algorithm>
#include <cstdint>
#include <sstream>

#include "evrp/sdk/eventformat.h"
#include "lua/luabindings.h"
#include "evrp/sdk/playbackeventcollector.h"

namespace api = evrp::device::api;

namespace {

int64_t eventTimeUs(const api::InputEvent& e) {
  return static_cast<int64_t>(e.timeSec) * 1000000LL +
         static_cast<int64_t>(e.timeUsec);
}

void setEventTimeUs(api::InputEvent& e, int64_t t_us) {
  e.timeSec = static_cast<decltype(e.timeSec)>(t_us / 1000000LL);
  e.timeUsec = static_cast<decltype(e.timeUsec)>(t_us % 1000000LL);
}

api::InputEvent recordFieldsToEvent(api::DeviceKind device, unsigned short type,
                                    unsigned short code, int value) {
  api::InputEvent e;
  e.device = device;
  e.timeSec = 0;
  e.timeUsec = 0;
  e.type = static_cast<uint32_t>(type);
  e.code = static_cast<uint32_t>(code);
  e.value = value;
  return e;
}

bool textLooksLikeRecordingFormat(const std::string& text) {
  std::istringstream in(text);
  std::string line;
  long long dus = 0;
  long long tmp = 0;
  unsigned short t = 0, c = 0;
  int v = 0;
  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }
    std::string label = parseEventLabel(line);
    if (label == "leading" && parseLeadingLine(line, &tmp)) {
      return true;
    }
    if (label == "trailing" && parseTrailingLine(line, &tmp)) {
      return true;
    }
    api::DeviceKind dev = api::deviceKindFromLabel(label);
    if (dev != api::DeviceKind::kUnspecified &&
        parseEventLine(line, &dus, &t, &c, &v)) {
      return true;
    }
  }
  return false;
}

void appendShiftedLuaBatch(std::vector<api::InputEvent>* out,
                           std::vector<api::InputEvent> batch, int64_t placement_us) {
  if (batch.empty() || !out) {
    return;
  }
  int64_t t_min = eventTimeUs(batch[0]);
  for (const auto& e : batch) {
    t_min = std::min(t_min, eventTimeUs(e));
  }
  const int64_t shift = placement_us - t_min;
  for (auto& e : batch) {
    setEventTimeUs(e, eventTimeUs(e) + shift);
    out->push_back(std::move(e));
  }
}

int64_t vectorMaxTimeUs(const std::vector<api::InputEvent>& v) {
  int64_t m = 0;
  for (const auto& e : v) {
    m = std::max(m, eventTimeUs(e));
  }
  return m;
}

int64_t vectorMinTimeUs(const std::vector<api::InputEvent>& v) {
  if (v.empty()) {
    return 0;
  }
  int64_t m = eventTimeUs(v[0]);
  for (const auto& e : v) {
    m = std::min(m, eventTimeUs(e));
  }
  return m;
}

void normalizeTimelineToZero(std::vector<api::InputEvent>* events) {
  if (!events || events->empty()) {
    return;
  }
  const int64_t t_min = vectorMinTimeUs(*events);
  for (auto& e : *events) {
    setEventTimeUs(e, eventTimeUs(e) - t_min);
  }
}

}

int EventComposer::toEvents(const std::string& text,
                             std::vector<api::InputEvent>* events) {
  if (!events) {
    return LUA_ERRRUN;
  }
  events->clear();

  if (!textLooksLikeRecordingFormat(text)) {
    PlaybackEventCollector collector;
    int err = evrp::lua::playbackLuaChunkIntoCollector(text.c_str(), &collector);
    if (err != LUA_OK) {
      return err;
    }
    *events = collector.takeEvents();
    normalizeTimelineToZero(events);
    return 0;
  }

  long long leading_delta_us = -1;
  int64_t file_anchor_us = 0;
  int64_t stream_max_us = 0;
  int64_t recorded_time_adjust_us = 0;
  bool had_lua = false;
  bool saw_first_recorded = false;

  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }

    std::string label = parseEventLabel(line);
    long long tmp = 0;
    if (label == "leading") {
      if (parseLeadingLine(line, &tmp)) {
        leading_delta_us = tmp;
      }
      continue;
    }
    if (label == "trailing") {
      (void)parseTrailingLine(line, &tmp);
      continue;
    }

    api::DeviceKind device = api::deviceKindFromLabel(label);
    long long deltaUs = 0;
    unsigned short type = 0, code = 0;
    int value = 0;
    bool is_event = (device != api::DeviceKind::kUnspecified) &&
                    parseEventLine(line, &deltaUs, &type, &code, &value);

    if (!is_event) {
      PlaybackEventCollector collector;
      int err = evrp::lua::playbackLuaChunkIntoCollector(line.c_str(), &collector);
      if (err != LUA_OK) {
        return err;
      }
      std::vector<api::InputEvent> batch = collector.takeEvents();
      const int64_t placement_us = std::max(stream_max_us, file_anchor_us);
      appendShiftedLuaBatch(events, std::move(batch), placement_us);
      stream_max_us = vectorMaxTimeUs(*events);
      had_lua = true;
      continue;
    }

    if (!saw_first_recorded) {
      if (had_lua && leading_delta_us > 0) {
        recorded_time_adjust_us = leading_delta_us;
      }
      saw_first_recorded = true;
    }

    api::InputEvent e = recordFieldsToEvent(device, type, code, value);
    setEventTimeUs(e, deltaUs + recorded_time_adjust_us);
    events->push_back(e);
    file_anchor_us = eventTimeUs(e);
    stream_max_us = std::max(stream_max_us, file_anchor_us);
  }

  if (!events->empty()) {
    normalizeTimelineToZero(events);
  }
  return 0;
}
