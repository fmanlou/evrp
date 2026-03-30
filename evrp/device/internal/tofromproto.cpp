#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::api {

evrp::device::v1::DeviceKind ToProto(DeviceKind k) {
  switch (k) {
    case DeviceKind::kTouchpad:
      return evrp::device::v1::DEVICE_KIND_TOUCHPAD;
    case DeviceKind::kTouchscreen:
      return evrp::device::v1::DEVICE_KIND_TOUCHSCREEN;
    case DeviceKind::kMouse:
      return evrp::device::v1::DEVICE_KIND_MOUSE;
    case DeviceKind::kKeyboard:
      return evrp::device::v1::DEVICE_KIND_KEYBOARD;
    default:
      return evrp::device::v1::DEVICE_KIND_UNSPECIFIED;
  }
}

DeviceKind FromProto(evrp::device::v1::DeviceKind k) {
  switch (k) {
    case evrp::device::v1::DEVICE_KIND_TOUCHPAD:
      return DeviceKind::kTouchpad;
    case evrp::device::v1::DEVICE_KIND_TOUCHSCREEN:
      return DeviceKind::kTouchscreen;
    case evrp::device::v1::DEVICE_KIND_MOUSE:
      return DeviceKind::kMouse;
    case evrp::device::v1::DEVICE_KIND_KEYBOARD:
      return DeviceKind::kKeyboard;
    default:
      return DeviceKind::kUnspecified;
  }
}

void FromProto(const google::protobuf::RepeatedField<int>& proto_kinds,
               std::vector<DeviceKind>* out) {
  const size_t base = out->size();
  out->reserve(base + static_cast<size_t>(proto_kinds.size()));
  for (int k : proto_kinds) {
    out->push_back(FromProto(static_cast<evrp::device::v1::DeviceKind>(k)));
  }
}

std::vector<DeviceKind> FromProto(
    const google::protobuf::RepeatedField<int>& proto_kinds) {
  std::vector<DeviceKind> kinds;
  FromProto(proto_kinds, &kinds);
  return kinds;
}

void ToProto(const std::vector<DeviceKind>& kinds,
             google::protobuf::RepeatedField<int>* proto_kinds) {
  for (DeviceKind k : kinds) {
    proto_kinds->Add(static_cast<int>(ToProto(k)));
  }
}

void ToProto(const InputEvent& e, evrp::device::v1::InputEvent* p) {
  p->set_device(ToProto(e.device));
  p->set_time_sec(e.time_sec);
  p->set_time_usec(e.time_usec);
  p->set_type(e.type);
  p->set_code(e.code);
  p->set_value(e.value);
}

void ToProto(
    const std::vector<InputEvent>& events,
    google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>* out) {
  for (const InputEvent& e : events) {
    ToProto(e, out->Add());
  }
}

void FromProto(const evrp::device::v1::InputEvent& p, InputEvent* e) {
  e->device = FromProto(p.device());
  e->time_sec = p.time_sec();
  e->time_usec = p.time_usec();
  e->type = p.type();
  e->code = p.code();
  e->value = p.value();
}

void FromProto(
    const google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>&
        proto_events,
    std::vector<InputEvent>* out) {
  const size_t base = out->size();
  out->reserve(base + static_cast<size_t>(proto_events.size()));
  for (int i = 0; i < proto_events.size(); ++i) {
    InputEvent e;
    FromProto(proto_events.Get(i), &e);
    out->push_back(std::move(e));
  }
}

std::vector<InputEvent> FromProto(
    const google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>&
        proto_events) {
  std::vector<InputEvent> events;
  FromProto(proto_events, &events);
  return events;
}

void ToProto(const OperationResult& r, evrp::device::v1::OperationResult* p) {
  p->set_code(r.code);
  p->set_message(r.message);
}

}  // namespace evrp::device::api
