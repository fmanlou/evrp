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

bool openAndGetCapabilities(const char *path, Capabilities *out);

int readEvents(int fd, Event *events, int max_count);

bool errnoIsEintr();

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
