#include "eventformat.h"

#include <linux/input-event-codes.h>

#include <sstream>
#include <string>

#include "deviceid.h"
#include "keyboard/keyboarddevice.h"

std::string parse_event_label(const std::string &line) {
  std::size_t lb = line.find('[');
  if (lb == std::string::npos) return "";
  std::size_t rb = line.find(']', lb + 1);
  if (rb == std::string::npos) return "";
  return line.substr(lb + 1, rb - lb - 1);
}

bool parse_event_line(const std::string &line, long long *out_delta_us,
                      unsigned short *out_type, unsigned short *out_code,
                      int *out_value) {
  if (!out_type || !out_code || !out_value) return false;

  std::size_t bracket = line.find("] ");
  if (bracket == std::string::npos) return false;
  std::size_t ts_start = bracket + 2;
  std::size_t ts_end = line.find(' ', ts_start);
  if (ts_end == std::string::npos || ts_end <= ts_start) return false;

  std::string ts_token = line.substr(ts_start, ts_end - ts_start);
  std::size_t dot = ts_token.find('.');
  if (dot == std::string::npos || dot == 0 || dot + 1 >= ts_token.size()) {
    return false;
  }
  long long sec = 0, usec = 0;
  try {
    sec = std::stoll(ts_token.substr(0, dot));
    usec = std::stoll(ts_token.substr(dot + 1));
  } catch (...) {
    return false;
  }
  if (out_delta_us) *out_delta_us = sec * 1000000LL + usec;

  std::size_t type_pos = line.find("type=", ts_end);
  if (type_pos == std::string::npos) return false;
  type_pos += 5;
  std::size_t type_end = line.find_first_of("( ", type_pos);
  if (type_end == std::string::npos) type_end = line.size();
  try {
    *out_type = static_cast<unsigned short>(
        std::stoul(line.substr(type_pos, type_end - type_pos)));
  } catch (...) {
    return false;
  }

  std::size_t code_pos = line.find("code=", type_end);
  if (code_pos == std::string::npos) return false;
  code_pos += 5;
  std::size_t code_end = line.find_first_of("( ", code_pos);
  if (code_end == std::string::npos) code_end = line.size();
  try {
    *out_code = static_cast<unsigned short>(
        std::stoul(line.substr(code_pos, code_end - code_pos)));
  } catch (...) {
    return false;
  }

  std::size_t value_pos = line.find("value=", code_end);
  if (value_pos == std::string::npos) return false;
  value_pos += 6;
  std::size_t value_end = line.find_first_of(" /", value_pos);
  if (value_end == std::string::npos) value_end = line.size();
  try {
    *out_value = std::stoi(line.substr(value_pos, value_end - value_pos));
  } catch (...) {
    return false;
  }
  return true;
}

std::string event_type_name(unsigned short type) {
  switch (type) {
    case EV_SYN:
      return "EV_SYN";
    case EV_KEY:
      return "EV_KEY";
    case EV_REL:
      return "EV_REL";
    case EV_ABS:
      return "EV_ABS";
    case EV_MSC:
      return "EV_MSC";
    default:
      return "EV_UNKNOWN";
  }
}

std::string event_code_name(unsigned short type, unsigned short code) {
  if (type == EV_MSC) {
    switch (code) {
      case MSC_SCAN:
        return "MSC_SCAN";
      case MSC_TIMESTAMP:
        return "MSC_TIMESTAMP";
      default:
        return "";
    }
  }
  if (type == EV_SYN) {
    switch (code) {
      case SYN_REPORT:
        return "SYN_REPORT";
      case SYN_MT_REPORT:
        return "SYN_MT_REPORT";
      default:
        return "";
    }
  }
  return "";
}

std::string format_event_line(DeviceId id, const Event &ev, long long delta_us) {
  std::ostringstream oss;
  std::string code_name = event_code_name(ev.type, ev.code);
  long long delta_sec = delta_us / 1000000LL;
  long long delta_usec = delta_us % 1000000LL;
  if (delta_usec < 0) {
    delta_sec -= 1;
    delta_usec += 1000000LL;
  }
  oss << "[" << device_label(id) << "] " << delta_sec << ".";
  oss.width(6);
  oss.fill('0');
  oss << delta_usec
      << " type=" << ev.type << "(" << event_type_name(ev.type) << ")"
      << " code=" << ev.code;
  if (!code_name.empty()) {
    oss << "(" << code_name << ")";
  }
  oss << " value=" << ev.value;
  if (id == DeviceId::Keyboard) {
    if (ev.type == EV_KEY) {
      oss << " // key=" << keyboard_key_name_from_code(ev.code)
          << " action=" << keyboard_key_action_from_value(ev.value);
    } else {
      oss << " // key=N/A action=non-key-event";
    }
  }
  return oss.str();
}
