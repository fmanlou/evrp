#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x) ((x) % BITS_PER_LONG)
#define BIT(x) (1UL << OFF(x))
#define TEST_BIT(bit, array) ((array[bit / BITS_PER_LONG] & BIT(bit)) != 0)

bool is_touchpad(const char* dev_path) {
  int fd = open(dev_path, O_RDONLY | O_NONBLOCK);
  if (fd < 0) return false;

  char name[256] = {0};
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    close(fd);
    return false;
  }

  unsigned long evbit[NBITS(EV_MAX)]{};
  unsigned long keybit[NBITS(KEY_MAX)]{};
  unsigned long absbit[NBITS(ABS_MAX)]{};

  if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0 ||
      ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0 ||
      ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) < 0) {
    close(fd);
    return false;
  }

  bool has_abs =
      TEST_BIT(EV_ABS, evbit) &&
      (TEST_BIT(ABS_X, absbit) || TEST_BIT(ABS_MT_POSITION_X, absbit));

  bool has_finger_tool = TEST_BIT(BTN_TOOL_FINGER, keybit) ||
                         TEST_BIT(BTN_TOOL_DOUBLETAP, keybit) ||
                         TEST_BIT(BTN_TOOL_TRIPLETAP, keybit);

  std::string n(name);
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  bool name_like_touchpad = n.find("touchpad") != std::string::npos ||
                            n.find("trackpad") != std::string::npos ||
                            n.find("synaptics") != std::string::npos ||
                            n.find("elan") != std::string::npos;

  close(fd);

  return has_abs && has_finger_tool && name_like_touchpad;
}

bool is_mouse(const char* dev_path) {
  int fd = open(dev_path, O_RDONLY | O_NONBLOCK);
  if (fd < 0) return false;

  char name[256] = {0};
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    close(fd);
    return false;
  }

  unsigned long evbit[NBITS(EV_MAX)]{};
  unsigned long keybit[NBITS(KEY_MAX)]{};
  unsigned long relbit[NBITS(REL_MAX)]{};

  if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0 ||
      ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0 ||
      ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit) < 0) {
    close(fd);
    return false;
  }

  bool has_rel = TEST_BIT(EV_REL, evbit) && TEST_BIT(REL_X, relbit) &&
                 TEST_BIT(REL_Y, relbit);

  bool has_buttons = TEST_BIT(BTN_LEFT, keybit) ||
                     TEST_BIT(BTN_RIGHT, keybit) ||
                     TEST_BIT(BTN_MIDDLE, keybit);

  std::string n(name);
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  bool name_like_mouse = n.find("mouse") != std::string::npos ||
                         n.find("trackball") != std::string::npos ||
                         n.find("pointer") != std::string::npos;

  close(fd);

  return has_rel && has_buttons && name_like_mouse;
}

bool is_keyboard(const char* dev_path) {
  int fd = open(dev_path, O_RDONLY | O_NONBLOCK);
  if (fd < 0) return false;

  char name[256] = {0};
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    close(fd);
    return false;
  }

  unsigned long evbit[NBITS(EV_MAX)]{};
  unsigned long keybit[NBITS(KEY_MAX)]{};

  if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0 ||
      ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) {
    close(fd);
    return false;
  }

  bool has_key = TEST_BIT(EV_KEY, evbit);

  bool has_keyboard_keys = TEST_BIT(KEY_ENTER, keybit) ||
                           TEST_BIT(KEY_SPACE, keybit) ||
                           TEST_BIT(KEY_ESC, keybit) || TEST_BIT(KEY_A, keybit);

  std::string n(name);
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  bool name_like_keyboard = n.find("keyboard") != std::string::npos ||
                            n.find("keypad") != std::string::npos;

  close(fd);

  return has_key && has_keyboard_keys && name_like_keyboard;
}

std::string find_first_touchpad() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_touchpad(dev.c_str())) {
      return dev;
    }
  }
  return {};
}

std::string find_first_mouse() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_mouse(dev.c_str())) {
      return dev;
    }
  }
  return {};
}

std::string find_first_keyboard() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_keyboard(dev.c_str())) {
      return dev;
    }
  }
  return {};
}

struct RecordTarget {
  int fd;
  std::string label;
  std::string path;
};

void record_events_multi(const std::vector<RecordTarget>& targets) {
  if (targets.empty()) return;

  std::vector<pollfd> pfds;
  pfds.reserve(targets.size());
  for (const auto& t : targets) {
    std::cout << "Recording " << t.label << " from " << t.path << std::endl;
    pfds.push_back({t.fd, POLLIN, 0});
  }
  std::cout << "(Ctrl+C to stop)" << std::endl;

  struct input_event events[64];
  while (true) {
    int ret = poll(pfds.data(), static_cast<nfds_t>(pfds.size()), -1);
    if (ret < 0) {
      std::perror("poll");
      break;
    }

    for (size_t i = 0; i < pfds.size(); ++i) {
      if (!(pfds[i].revents & POLLIN)) continue;

      ssize_t n = read(targets[i].fd, events, sizeof(events));
      if (n <= 0) {
        if (n < 0) std::perror("read");
        return;
      }

      int count = static_cast<int>(n / sizeof(struct input_event));
      for (int j = 0; j < count; ++j) {
        const auto& ev = events[j];
        if (ev.type == EV_SYN) continue;

        std::cout << "[" << targets[i].label << "] " << ev.time.tv_sec << "."
                  << ev.time.tv_usec << " type=" << ev.type
                  << " code=" << ev.code << " value=" << ev.value << std::endl;
      }
    }
  }
}

void print_usage(const char* prog) {
  std::cout << "Usage: " << prog << " -r [touchpad] [mouse] [keyboard] ...\n"
            << "  -r: start recording. With no types, record touchpad, mouse, keyboard.\n";
}

bool parse_kind(const std::string& s, std::string* out_label) {
  if (s == "touchpad" || s == "mouse" || s == "keyboard") {
    *out_label = s;
    return true;
  }
  return false;
}

int main(int argc, char* argv[]) {
  if (argc < 2 || std::string(argv[1]) != "-r") {
    print_usage(argv[0]);
    return 1;
  }

  std::vector<std::string> kinds;
  for (int i = 2; i < argc; ++i) {
    std::string label;
    if (!parse_kind(argv[i], &label)) {
      std::cout << "Unknown device type: " << argv[i] << std::endl;
      print_usage(argv[0]);
      return 1;
    }
    kinds.push_back(label);
  }
  if (kinds.empty())
    kinds = {"touchpad", "mouse", "keyboard"};

  std::vector<RecordTarget> targets;
  for (const auto& kind : kinds) {
    std::string path;
    if (kind == "touchpad")
      path = find_first_touchpad();
    else if (kind == "mouse")
      path = find_first_mouse();
    else
      path = find_first_keyboard();

    if (path.empty()) {
      std::cout << "No " << kind << " detected. Try running with sudo."
                << std::endl;
      continue;
    }

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
      std::perror(path.c_str());
      continue;
    }
    targets.push_back({fd, kind, path});
  }

  if (targets.empty()) {
    std::cout << "No devices to record." << std::endl;
    return 1;
  }

  record_events_multi(targets);

  for (const auto& t : targets) close(t.fd);
  return 0;
}