#include "evrp/device/api/deviceprotoconv.h"

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
  out->clear();
  out->reserve(static_cast<size_t>(proto_kinds.size()));
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

void ToProto(const InputEvent& e, evrp::device::v1::InputEvent* p) {
  p->set_device(ToProto(e.device));
  p->set_time_sec(e.time_sec);
  p->set_time_usec(e.time_usec);
  p->set_type(e.type);
  p->set_code(e.code);
  p->set_value(e.value);
}

void FromProto(const evrp::device::v1::InputEvent& p, InputEvent* e) {
  e->device = FromProto(p.device());
  e->time_sec = p.time_sec();
  e->time_usec = p.time_usec();
  e->type = p.type();
  e->code = p.code();
  e->value = p.value();
}

}  // namespace evrp::device::api
