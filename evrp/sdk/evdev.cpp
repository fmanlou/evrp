#include "evrp/sdk/evdev.h"

#include <linux/input.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <cerrno>
#include <cstring>

#include "evrp/sdk/filesystem.h"

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define NBITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x) ((x) % BITS_PER_LONG)
#define BIT(x) (1UL << OFF(x))
#define TEST_BIT(bit, array) ((array[(bit) / BITS_PER_LONG] & BIT(bit)) != 0)

bool openAndGetCapabilities(const char *path, Capabilities *caps) {
  FileSystem fs;
  int fd = fs.openReadOnly(path, true);
  if (fd < 0) return false;
  bool ok = getCapabilities(fd, caps);
  fs.closeFd(fd);
  return ok;
}

bool getCapabilities(int fd, Capabilities *caps) {
  if (fd < 0 || !caps) return false;

  char name[256] = {0};
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) return false;

  caps->name.assign(name);

  unsigned long evbit[NBITS(EV_MAX)]{};
  unsigned long keybit[NBITS(KEY_MAX)]{};
  unsigned long absbit[NBITS(ABS_MAX)]{};
  unsigned long relbit[NBITS(REL_MAX)]{};

  if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0) return false;
  if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) return false;
  if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) < 0) return false;
  if (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit) < 0) return false;

  caps->evKey = TEST_BIT(EV_KEY, evbit);
  caps->evAbs = TEST_BIT(EV_ABS, evbit);
  caps->evRel = TEST_BIT(EV_REL, evbit);
  caps->absX = TEST_BIT(ABS_X, absbit);
  caps->absMtPositionX = TEST_BIT(ABS_MT_POSITION_X, absbit);
  caps->relX = TEST_BIT(REL_X, relbit);
  caps->relY = TEST_BIT(REL_Y, relbit);
  caps->btnLeft = TEST_BIT(BTN_LEFT, keybit);
  caps->btnRight = TEST_BIT(BTN_RIGHT, keybit);
  caps->btnMiddle = TEST_BIT(BTN_MIDDLE, keybit);
  caps->btnToolFinger = TEST_BIT(BTN_TOOL_FINGER, keybit);
  caps->btnToolDoubletap = TEST_BIT(BTN_TOOL_DOUBLETAP, keybit);
  caps->btnToolTripletap = TEST_BIT(BTN_TOOL_TRIPLETAP, keybit);
  caps->keyEnter = TEST_BIT(KEY_ENTER, keybit);
  caps->keySpace = TEST_BIT(KEY_SPACE, keybit);
  caps->keyEsc = TEST_BIT(KEY_ESC, keybit);
  caps->keyA = TEST_BIT(KEY_A, keybit);

  return true;
}

int readEvents(int fd, Event *events, int max_count) {
  if (fd < 0 || !events || max_count <= 0) return -1;
  FileSystem fs;

  struct input_event raw[64];
  long n = fs.readFd(fd, raw, sizeof(raw));
  if (n <= 0) return static_cast<int>(n);
  if (n % sizeof(struct input_event) != 0) return -1;

  int count = static_cast<int>(n / sizeof(struct input_event));
  int out = 0;
  for (int i = 0; i < count && out < max_count; ++i) {
    events[out].sec = raw[i].time.tv_sec;
    events[out].usec = raw[i].time.tv_usec;
    events[out].type = raw[i].type;
    events[out].code = raw[i].code;
    events[out].value = raw[i].value;
    ++out;
  }
  return out;
}

volatile sig_atomic_t SigintGuard::stop_ = 0;

void SigintGuard::handler(int) { stop_ = 1; }

SigintGuard::SigintGuard() {
  stop_ = 0;
  struct sigaction sa = {};
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, &oldSa_);
}

SigintGuard::~SigintGuard() { sigaction(SIGINT, &oldSa_, nullptr); }

bool SigintGuard::stopRequested() const { return stop_ != 0; }

bool errnoIsEintr() { return errno == EINTR; }
