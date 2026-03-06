#ifndef EVRP_EVDEV_H
#define EVRP_EVDEV_H

#include <string>

namespace evdev {

struct Capabilities {
  std::string name;
  bool ev_key;
  bool ev_abs;
  bool ev_rel;
  bool abs_x;
  bool abs_mt_position_x;
  bool rel_x;
  bool rel_y;
  bool btn_left;
  bool btn_right;
  bool btn_middle;
  bool btn_tool_finger;
  bool btn_tool_doubletap;
  bool btn_tool_tripletap;
  bool key_enter;
  bool key_space;
  bool key_esc;
  bool key_a;
};

struct Event {
  long sec;
  long usec;
  unsigned short type;
  unsigned short code;
  int value;
};

// Event type constant (EV_SYN)
constexpr unsigned short EV_SYN = 0;

// Device handle: -1 means invalid
int open(const char* path, bool nonblocking = false);
void close(int fd);
bool get_capabilities(int fd, Capabilities* out);

// Convenience: open nonblocking, get caps, close. Returns true on success.
bool open_and_get_capabilities(const char* path, Capabilities* out);

// Returns number of events read, or -1 on error
int read_events(int fd, Event* events, int max_count);

// Poll: fills ready[i] for each fd with data. Returns n_ready, 0 on timeout, -1 on error.
int poll(int* fds, int nfds, int timeout_ms, bool* ready);

// After poll returns -1, check if errno was EINTR
bool errno_is_eintr();
void perror(const char* msg);

// Signal handling for graceful shutdown
void signal_install_sigint();
void signal_restore_sigint();
bool signal_stop_requested();

}  // namespace evdev

#endif  // EVRP_EVDEV_H
