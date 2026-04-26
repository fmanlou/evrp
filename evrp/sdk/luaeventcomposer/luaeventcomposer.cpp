#include "evrp/sdk/luaeventcomposer/luaeventcomposer.h"

#include <algorithm>
#include <cstdint>
#include <sstream>

#include "evrp/sdk/eventformat.h"
#include "evrp/sdk/luaeventcomposer/luabindings.h"
#include "evrp/sdk/playbackeventcollector.h"

extern "C" {
#include "lua.h"
}

namespace api = evrp::device::api;

namespace {

int64_t eventTimeUs(const api::InputEvent& e) {
  return static_cast<int64_t>(e.timeSec) * 1000000LL +
         static_cast<int64_t>(e.timeUsec);
}

void setEventTimeUs(api::InputEvent& e, int64_t tUs) {
  e.timeSec = static_cast<decltype(e.timeSec)>(tUs / 1000000LL);
  e.timeUsec = static_cast<decltype(e.timeUsec)>(tUs % 1000000LL);
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
                           std::vector<api::InputEvent> batch, int64_t placementUs) {
  if (batch.empty() || !out) {
    return;
  }
  int64_t tMin = eventTimeUs(batch[0]);
  for (const auto& e : batch) {
    tMin = std::min(tMin, eventTimeUs(e));
  }
  const int64_t shift = placementUs - tMin;
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
  const int64_t tMin = vectorMinTimeUs(*events);
  for (auto& e : *events) {
    setEventTimeUs(e, eventTimeUs(e) - tMin);
  }
}

}

int LuaEventComposer::toEvents(const std::string& text,
                               std::vector<api::InputEvent>* events) {
  if (!events) {
    return LUA_ERRRUN;
  }
  events->clear();

  if (!textLooksLikeRecordingFormat(text)) {
    PlaybackEventCollector collector;
    int err =
        evrp::lua::playbackLuaChunkIntoCollector(text.c_str(), &collector);
    if (err != LUA_OK) {
      return err;
    }
    *events = collector.takeEvents();
    normalizeTimelineToZero(events);
    return 0;
  }

  long long leadingDeltaUs = -1;
  int64_t fileAnchorUs = 0;
  int64_t streamMaxUs = 0;
  int64_t recordedTimeAdjustUs = 0;
  bool hadLua = false;
  bool sawFirstRecorded = false;

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
        leadingDeltaUs = tmp;
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
    bool isEvent = (device != api::DeviceKind::kUnspecified) &&
                    parseEventLine(line, &deltaUs, &type, &code, &value);

    if (!isEvent) {
      PlaybackEventCollector collector;
      int err =
          evrp::lua::playbackLuaChunkIntoCollector(line.c_str(), &collector);
      if (err != LUA_OK) {
        return err;
      }
      std::vector<api::InputEvent> batch = collector.takeEvents();
      const int64_t placementUs = std::max(streamMaxUs, fileAnchorUs);
      appendShiftedLuaBatch(events, std::move(batch), placementUs);
      streamMaxUs = vectorMaxTimeUs(*events);
      hadLua = true;
      continue;
    }

    if (!sawFirstRecorded) {
      if (hadLua && leadingDeltaUs > 0) {
        recordedTimeAdjustUs = leadingDeltaUs;
      }
      sawFirstRecorded = true;
    }

    api::InputEvent e = recordFieldsToEvent(device, type, code, value);
    setEventTimeUs(e, deltaUs + recordedTimeAdjustUs);
    events->push_back(e);
    fileAnchorUs = eventTimeUs(e);
    streamMaxUs = std::max(streamMaxUs, fileAnchorUs);
  }

  if (!events->empty()) {
    normalizeTimelineToZero(events);
  }
  return 0;
}
