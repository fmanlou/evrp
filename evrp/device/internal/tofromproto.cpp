#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::api {
namespace v1 = evrp::device::v1;

v1::DeviceKind toProto(DeviceKind k) {
  switch (k) {
    case DeviceKind::kTouchpad:
      return v1::DEVICE_KIND_TOUCHPAD;
    case DeviceKind::kTouchscreen:
      return v1::DEVICE_KIND_TOUCHSCREEN;
    case DeviceKind::kMouse:
      return v1::DEVICE_KIND_MOUSE;
    case DeviceKind::kKeyboard:
      return v1::DEVICE_KIND_KEYBOARD;
    default:
      return v1::DEVICE_KIND_UNSPECIFIED;
  }
}

DeviceKind fromProto(v1::DeviceKind k) {
  switch (k) {
    case v1::DEVICE_KIND_TOUCHPAD:
      return DeviceKind::kTouchpad;
    case v1::DEVICE_KIND_TOUCHSCREEN:
      return DeviceKind::kTouchscreen;
    case v1::DEVICE_KIND_MOUSE:
      return DeviceKind::kMouse;
    case v1::DEVICE_KIND_KEYBOARD:
      return DeviceKind::kKeyboard;
    default:
      return DeviceKind::kUnspecified;
  }
}

void fromProto(const google::protobuf::RepeatedField<int>& protoKinds,
               std::vector<DeviceKind>* out) {
  const size_t base = out->size();
  out->reserve(base + static_cast<size_t>(protoKinds.size()));
  for (int k : protoKinds) {
    out->push_back(fromProto(static_cast<v1::DeviceKind>(k)));
  }
}

std::vector<DeviceKind> fromProto(
    const google::protobuf::RepeatedField<int>& protoKinds) {
  std::vector<DeviceKind> kinds;
  fromProto(protoKinds, &kinds);
  return kinds;
}

void toProto(const std::vector<DeviceKind>& kinds,
             google::protobuf::RepeatedField<int>* protoKinds) {
  for (DeviceKind k : kinds) {
    protoKinds->Add(static_cast<int>(toProto(k)));
  }
}

void toProto(const InputEvent& e, v1::InputEvent* p) {
  p->set_device(toProto(e.device));
  p->set_time_sec(e.timeSec);
  p->set_time_usec(e.timeUsec);
  p->set_type(e.type);
  p->set_code(e.code);
  p->set_value(e.value);
}

void toProto(
    const std::vector<InputEvent>& events,
    google::protobuf::RepeatedPtrField<v1::InputEvent>* out) {
  for (const InputEvent& e : events) {
    toProto(e, out->Add());
  }
}

void fromProto(const v1::InputEvent& p, InputEvent* e) {
  e->device = fromProto(p.device());
  e->timeSec = p.time_sec();
  e->timeUsec = p.time_usec();
  e->type = p.type();
  e->code = p.code();
  e->value = p.value();
}

void fromProto(
    const google::protobuf::RepeatedPtrField<v1::InputEvent>& protoEvents,
    std::vector<InputEvent>* out) {
  const size_t base = out->size();
  out->reserve(base + static_cast<size_t>(protoEvents.size()));
  for (int i = 0; i < protoEvents.size(); ++i) {
    InputEvent e;
    fromProto(protoEvents.Get(i), &e);
    out->push_back(std::move(e));
  }
}

std::vector<InputEvent> fromProto(
    const google::protobuf::RepeatedPtrField<v1::InputEvent>& protoEvents) {
  std::vector<InputEvent> events;
  fromProto(protoEvents, &events);
  return events;
}

void toProto(const OperationResult& r, v1::OperationResult* p) {
  p->set_code(r.code);
  p->set_message(r.message);
}

}

