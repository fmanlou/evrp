#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

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

int main() {
  std::cout << "Scanning /dev/input/eventX for touchpad devices..."
            << std::endl;

  bool found = false;
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_touchpad(dev.c_str())) {
      std::cout << "Detected touchpad device: " << dev << std::endl;
      found = true;
    }
  }

  if (!found) {
    std::cout << "No touchpad detected. Try running with sudo." << std::endl;
  }

  return 0;
}