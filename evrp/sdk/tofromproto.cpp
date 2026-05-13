#include "evrp/sdk/tofromproto.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <typeinfo>
#include <vector>

#include "evrp/sdk/tofromstring.h"
#include "evrp/sdk/logger.h"

#include <google/protobuf/struct.pb.h>

namespace evrp::sdk {
namespace {

std::optional<std::any> valueFromProto(const google::protobuf::Value& v) {
  using google::protobuf::Value;
  switch (v.kind_case()) {
    case Value::kNullValue:
      return std::nullopt;
    case Value::kNumberValue: {
      const double n = v.number_value();
      if (std::isfinite(n) && n == std::trunc(n) &&
          n >= static_cast<double>(std::numeric_limits<int32_t>::min()) &&
          n <= static_cast<double>(std::numeric_limits<int32_t>::max())) {
        return std::any(static_cast<int32_t>(static_cast<long>(n)));
      }
      return std::any(n);
    }
    case Value::kStringValue:
      return std::any(v.string_value());
    case Value::kBoolValue:
      return std::any(v.bool_value());
    case Value::kStructValue:
    case Value::kListValue:
    case Value::KIND_NOT_SET:
    default:
      return std::nullopt;
  }
}

bool appendKindsFromList(std::map<std::string, std::any>& out,
                         const google::protobuf::ListValue& list) {
  std::vector<evrp::device::api::DeviceKind> kinds;
  for (const google::protobuf::Value& v : list.values()) {
    if (v.kind_case() != google::protobuf::Value::kStringValue) {
      continue;
    }
    const auto k = evrp::device::api::toKind(v.string_value());
    if (k != evrp::device::api::DeviceKind::kUnspecified) {
      kinds.push_back(k);
    }
  }
  if (kinds.empty()) {
    return false;
  }
  out.insert_or_assign("kinds", std::move(kinds));
  return true;
}

bool anyToProtoValue(const std::any& a, google::protobuf::Value* out) {
  using evrp::device::api::DeviceKind;
  using evrp::device::api::toString;
  using logging::LogLevel;

  const std::type_info& ti = a.type();
  if (ti == typeid(std::string)) {
    out->set_string_value(std::any_cast<const std::string&>(a));
    return true;
  }
  if (ti == typeid(bool)) {
    out->set_bool_value(std::any_cast<bool>(a));
    return true;
  }
  if (ti == typeid(int32_t)) {
    out->set_number_value(static_cast<double>(std::any_cast<int32_t>(a)));
    return true;
  }
  if (ti == typeid(int64_t)) {
    out->set_number_value(static_cast<double>(std::any_cast<int64_t>(a)));
    return true;
  }
  if (ti == typeid(double)) {
    out->set_number_value(std::any_cast<double>(a));
    return true;
  }
  if (ti == typeid(float)) {
    out->set_number_value(static_cast<double>(std::any_cast<float>(a)));
    return true;
  }
  if (ti == typeid(LogLevel)) {
    out->set_string_value(logLevelName(std::any_cast<LogLevel>(a)));
    return true;
  }
  if (ti == typeid(std::vector<DeviceKind>)) {
    google::protobuf::ListValue* lv = out->mutable_list_value();
    for (DeviceKind dk : std::any_cast<const std::vector<DeviceKind>&>(a)) {
      lv->add_values()->set_string_value(toString(dk));
    }
    return true;
  }
  if (ti == typeid(std::vector<std::string>)) {
    google::protobuf::ListValue* lv = out->mutable_list_value();
    for (const std::string& s : std::any_cast<const std::vector<std::string>&>(a)) {
      lv->add_values()->set_string_value(s);
    }
    return true;
  }
  return false;
}

}  // namespace

void fromProto(std::map<std::string, std::any>& out,
               const google::protobuf::Struct& s) {
  for (const auto& entry : s.fields()) {
    const std::string& key = entry.first;
    const google::protobuf::Value& val = entry.second;
    if (key == "kinds" &&
        val.kind_case() == google::protobuf::Value::kListValue) {
      appendKindsFromList(out, val.list_value());
      continue;
    }
    if (key == "logLevel" &&
        val.kind_case() == google::protobuf::Value::kStringValue) {
      out.insert_or_assign(key, logLevelFromString(val.string_value()));
      continue;
    }
    std::optional<std::any> converted = valueFromProto(val);
    if (!converted.has_value()) {
      continue;
    }
    out.insert_or_assign(key, std::move(*converted));
  }
}

void toProto(const std::map<std::string, std::any>& snap,
             google::protobuf::Struct* out) {
  if (!out) {
    return;
  }
  google::protobuf::Struct result;
  for (const auto& entry : snap) {
    if (!entry.second.has_value()) {
      continue;
    }
    google::protobuf::Value v;
    if (!anyToProtoValue(entry.second, &v)) {
      continue;
    }
    (*result.mutable_fields())[entry.first] = std::move(v);
  }
  *out = std::move(result);
}

}  // namespace evrp::sdk
