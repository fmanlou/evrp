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

int main() {
  std::cout << "Scanning /dev/input/eventX for input devices..." << std::endl;

  bool found_any = false;
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_touchpad(dev.c_str())) {
      std::cout << "  Touchpad: " << dev << std::endl;
      found_any = true;
    }
    if (is_mouse(dev.c_str())) {
      std::cout << "  Mouse:    " << dev << std::endl;
      found_any = true;
    }
    if (is_keyboard(dev.c_str())) {
      std::cout << "  Keyboard: " << dev << std::endl;
      found_any = true;
    }
  }

  if (!found_any) {
    std::cout << "No input devices detected. Try running with sudo."
              << std::endl;
  }

  return 0;
}