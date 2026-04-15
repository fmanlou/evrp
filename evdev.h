#pragma once

#include <csignal>
#include <string>

struct Capabilities {
  std::string name;
  bool evKey;
  bool evAbs;
  bool evRel;
  bool absX;
  bool absMtPositionX;
  bool relX;
  bool relY;
  bool btnLeft;
  bool btnRight;
  bool btnMiddle;
  bool btnToolFinger;
  bool btnToolDoubletap;
  bool btnToolTripletap;
  bool keyEnter;
  bool keySpace;
  bool keyEsc;
  bool keyA;
};

struct Event {
  long sec;
  long usec;
  unsigned short type;
  unsigned short code;
  int value;
};

bool getCapabilities(int fd, Capabilities *out);

// Convenience: open nonblocking, get caps, close. Returns true on success.
bool openAndGetCapabilities(const char *path, Capabilities *out);

// Returns number of events read, or -1 on error
int readEvents(int fd, Event *events, int max_count);

// After poll returns -1, check if errno was EINTR
bool errnoIsEintr();

// RAII: installs SIGINT handler in ctor, restores in dtor
class SigintGuard {
 public:
  SigintGuard();
  ~SigintGuard();
  bool stopRequested() const;

 private:
  struct sigaction oldSa_;
  static volatile sig_atomic_t stop_;
  static void handler(int);
};
