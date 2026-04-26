#include "eventformat.h"

#include <linux/input-event-codes.h>

#include <sstream>
#include <string>

#include "keyboard/keyboarddevice.h"

std::string parseEventLabel(const std::string &line) {
  std::size_t lb = line.find('[');
  if (lb == std::string::npos) return "";
  std::size_t rb = line.find(']', lb + 1);
  if (rb == std::string::npos) return "";
  return line.substr(lb + 1, rb - lb - 1);
}

bool parseEventLine(const std::string &line, long long *out_deltaUs,
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
  if (out_deltaUs) *out_deltaUs = sec * 1000000LL + usec;

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

static bool parseLabeledDurationLine(const std::string &line,
                                        const std::string &expected_label,
                                        long long *out_deltaUs) {
  if (!out_deltaUs) return false;
  std::string label = parseEventLabel(line);
  if (label != expected_label) return false;
  std::size_t bracket = line.find("] ");
  if (bracket == std::string::npos) return false;
  std::size_t ts_start = bracket + 2;
  std::string ts_token = line.substr(ts_start);
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
  *out_deltaUs = sec * 1000000LL + usec;
  return true;
}

bool parseLeadingLine(const std::string &line, long long *out_deltaUs) {
  return parseLabeledDurationLine(line, "leading", out_deltaUs);
}

bool parseTrailingLine(const std::string &line, long long *out_deltaUs) {
  return parseLabeledDurationLine(line, "trailing", out_deltaUs);
}

std::string eventTypeName(unsigned short type) {
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

std::string eventCodeName(unsigned short type, unsigned short code) {
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

std::string formatEventLine(evrp::device::api::DeviceKind device,
                            const Event &ev, long long deltaUs) {
  std::ostringstream oss;
  std::string code_name = eventCodeName(ev.type, ev.code);
  long long delta_sec = deltaUs / 1000000LL;
  long long deltaUsec = deltaUs % 1000000LL;
  if (deltaUsec < 0) {
    delta_sec -= 1;
    deltaUsec += 1000000LL;
  }
  oss << "[" << evrp::device::api::deviceKindLabel(device) << "] " << delta_sec
      << ".";
  oss.width(6);
  oss.fill('0');
  oss << deltaUsec
      << " type=" << ev.type << "(" << eventTypeName(ev.type) << ")"
      << " code=" << ev.code;
  if (!code_name.empty()) {
    oss << "(" << code_name << ")";
  }
  oss << " value=" << ev.value;
  if (device == evrp::device::api::DeviceKind::kKeyboard) {
    if (ev.type == EV_KEY) {
      oss << " // key=" << keyboardKeyNameFromCode(ev.code)
          << " action=" << keyboardKeyActionFromValue(ev.value);
    } else {
      oss << " // key=N/A action=non-key-event";
    }
  }
  return oss.str();
}

static std::string formatDurationLine(const std::string &label,
                                        long long deltaUs) {
  std::ostringstream oss;
  long long delta_sec = deltaUs / 1000000LL;
  long long deltaUsec = deltaUs % 1000000LL;
  if (deltaUsec < 0) {
    delta_sec -= 1;
    deltaUsec += 1000000LL;
  }
  oss << "[" << label << "] " << delta_sec << ".";
  oss.width(6);
  oss.fill('0');
  oss << deltaUsec;
  return oss.str();
}

std::string formatLeadingLine(long long deltaUs) {
  return formatDurationLine("leading", deltaUs);
}

std::string formatTrailingLine(long long deltaUs) {
  return formatDurationLine("trailing", deltaUs);
}
