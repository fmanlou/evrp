#include <gflags/gflags.h>
#include <grpcpp/grpcpp.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "evrp/device/api/countingsemaphore.h"
#include "evrp/device/api/deviceclient.h"
#include "evrp/device/api/types.h"
#include "logger.h"

DEFINE_string(
    target, "",
    "evrp-device on the device side as host:port (e.g. 192.168.1.10:50051). "
    "If set, overrides --host/--port.");
DEFINE_string(host, "",
              "Device hostname or IP; use with --port when --target is empty");
DEFINE_int32(port, 50051,
             "TCP port on the device; used with --host when --target is empty");
DEFINE_string(device_binary, "",
              "Optional: path to evrp-device to spawn locally on 127.0.0.1 "
              "(CI). If set, --target/--host/--port are ignored for the "
              "connection. Or set EVRP_DEVICE_BINARY.");
DEFINE_int32(
    rpc_wait_ms, 15000,
    "Maximum time to wait for Ping after connect attempts begin (ms)");
DEFINE_bool(test_input_listen, true,
            "Exercise InputListenService (StartRecording / Wait / Read / Stop)");
DEFINE_bool(test_playback, true,
            "Exercise PlaybackService when device reports keyboard capability");
DEFINE_int32(listen_wait_ms, 400,
             "WaitForInputEvent timeout (ms) while testing InputListen; must "
             "be >= 0");

namespace {

int pickFreeLoopbackPort() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = 0;
  if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
    close(fd);
    return -1;
  }
  socklen_t len = sizeof(addr);
  if (getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
    close(fd);
    return -1;
  }
  int port = ntohs(addr.sin_port);
  close(fd);
  return port;
}

std::string deviceBinaryPath() {
  if (!FLAGS_device_binary.empty()) {
    return FLAGS_device_binary;
  }
  const char* env = std::getenv("EVRP_DEVICE_BINARY");
  return env ? std::string(env) : std::string();
}

bool isExecutableFile(const std::string& path) {
  struct stat st {};
  if (stat(path.c_str(), &st) != 0) {
    return false;
  }
  if (!S_ISREG(st.st_mode)) {
    return false;
  }
  return access(path.c_str(), X_OK) == 0;
}

// Returns address or empty; sets *ok false on invalid --host/--port combo.
std::string remoteTargetFromFlags(bool* ok) {
  *ok = true;
  if (!FLAGS_target.empty()) {
    return FLAGS_target;
  }
  if (FLAGS_host.empty()) {
    return "";
  }
  if (FLAGS_port <= 0 || FLAGS_port > 65535) {
    *ok = false;
    return "";
  }
  return FLAGS_host + ":" + std::to_string(FLAGS_port);
}

bool waitForPing(const std::shared_ptr<grpc::Channel>& channel,
                 int totalTimeoutMs) {
  const std::unique_ptr<evrp::device::api::IInputDeviceClient> device =
      evrp::device::api::makeRemoteInputDeviceClient(channel);
  const auto overall =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(totalTimeoutMs);
  while (std::chrono::steady_clock::now() < overall) {
    if (device->ping()) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
  return false;
}

bool fetchCapabilities(
    const std::shared_ptr<grpc::Channel>& channel,
    std::vector<evrp::device::api::DeviceKind>* kindsOut) {
  kindsOut->clear();
  const std::unique_ptr<evrp::device::api::IInputDeviceClient> device =
      evrp::device::api::makeRemoteInputDeviceClient(channel);
  if (!device->getCapabilities(kindsOut)) {
    logError("GetCapabilities failed");
    return false;
  }
  logInfo("GetCapabilities ok (" +
          std::to_string(kindsOut->size()) + " kind(s))");
  return true;
}

std::vector<evrp::device::api::DeviceKind> kindsForRecording(
    const std::vector<evrp::device::api::DeviceKind>& caps) {
  std::vector<evrp::device::api::DeviceKind> out;
  for (evrp::device::api::DeviceKind k : caps) {
    if (k != evrp::device::api::DeviceKind::kUnspecified) {
      out.push_back(k);
    }
  }
  return out;
}

bool deviceHasKeyboard(
    const std::vector<evrp::device::api::DeviceKind>& caps) {
  for (evrp::device::api::DeviceKind k : caps) {
    if (k == evrp::device::api::DeviceKind::kKeyboard) {
      return true;
    }
  }
  return false;
}

std::vector<evrp::device::api::InputEvent> minimalKeyTapEvents() {
  std::vector<evrp::device::api::InputEvent> v;
  int usec = 0;
  for (int value : {1, 0}) {
    evrp::device::api::InputEvent e;
    e.device = evrp::device::api::DeviceKind::kKeyboard;
    e.timeSec = 0;
    e.timeUsec = usec;
    e.type = static_cast<uint32_t>(EV_KEY);
    e.code = static_cast<uint32_t>(KEY_A);
    e.value = value;
    v.push_back(e);
    usec += 1000;
  }
  return v;
}

bool testInputListen(
    const std::shared_ptr<grpc::Channel>& channel,
    const std::vector<evrp::device::api::DeviceKind>& caps) {
  if (!FLAGS_test_input_listen) {
    logInfo("InputListen: skipped (--test_input_listen=false)");
    return true;
  }
  const std::vector<evrp::device::api::DeviceKind> kinds =
      kindsForRecording(caps);
  if (kinds.empty()) {
    logInfo(
        "InputListen: skip (no input device kinds on device; recording N/A)");
    return true;
  }
  if (FLAGS_listen_wait_ms < 0) {
    logError("--listen_wait_ms must be >= 0");
    return false;
  }

  const std::unique_ptr<evrp::device::api::IInputListener> listener =
      evrp::device::api::makeRemoteInputListener(channel);
  if (!listener->startListening(kinds)) {
    logError(
        "InputListen: StartRecording failed (no matching devices or error)");
    return false;
  }
  (void)listener->waitForInputEvent(FLAGS_listen_wait_ms);
  const std::vector<evrp::device::api::InputEvent> batch =
      listener->readInputEvents();
  logInfo("InputListen: ReadInputEvents count=" +
          std::to_string(batch.size()));
  listener->cancelListening();
  logInfo("InputListen: StopRecording ok");
  return true;
}

bool testPlayback(const std::shared_ptr<grpc::Channel>& channel,
                  const std::vector<evrp::device::api::DeviceKind>& caps) {
  if (!FLAGS_test_playback) {
    logInfo("Playback: skipped (--test_playback=false)");
    return true;
  }
  if (!deviceHasKeyboard(caps)) {
    logInfo(
        "Playback: skip (device has no keyboard capability; upload/playback "
        "N/A)");
    return true;
  }

  const auto events = minimalKeyTapEvents();
  const std::unique_ptr<evrp::device::api::IPlayback> playback =
      evrp::device::api::makeRemotePlayback(channel);
  evrp::device::api::OperationResult up;
  if (!playback->upload(events, &up)) {
    logError("Playback: Upload failed code=" + std::to_string(up.code) +
             " msg=" + up.message);
    return false;
  }
  logInfo("Playback: Upload ok (" + std::to_string(events.size()) + " events)");

  evrp::CountingSemaphore progressSem;
  const int n = static_cast<int>(events.size());
  std::thread consumer([&]() {
    for (int i = 0; i < n; ++i) {
      progressSem.acquire();
    }
  });

  evrp::device::api::OperationResult play;
  const bool ok = playback->playback(&play, &progressSem);
  consumer.join();

  if (!ok) {
    logError("Playback: Playback failed code=" + std::to_string(play.code) +
             " msg=" + play.message);
    return false;
  }
  logInfo("Playback: Playback RPC finished ok");
  return true;
}

struct DeviceProcess {
  pid_t pid = -1;

  ~DeviceProcess() {
    if (pid <= 0) {
      return;
    }
    kill(pid, SIGTERM);
    int st = 0;
    for (int i = 0; i < 200; ++i) {
      pid_t r = waitpid(pid, &st, WNOHANG);
      if (r == pid) {
        pid = -1;
        return;
      }
      if (r < 0) {
        pid = -1;
        return;
      }
      usleep(50000);
    }
    kill(pid, SIGKILL);
    waitpid(pid, &st, 0);
    pid = -1;
  }
};

}  // namespace

int main(int argc, char** argv) {
  Logger logger;
  gLogger = &logger;

  gflags::SetUsageMessage(
      "Host-side check: Ping, GetCapabilities, InputListen, Playback against "
      "evrp-device (--target or --host/--port; optional --device_binary for "
      "CI)");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::string target;
  DeviceProcess proc;

  const std::string binary = deviceBinaryPath();
  if (!binary.empty()) {
    if (!isExecutableFile(binary)) {
      logError("Not an executable file: " + binary);
      return 2;
    }
    const int ephemeral = pickFreeLoopbackPort();
    if (ephemeral <= 0) {
      logError("pickFreeLoopbackPort failed (bind loopback ephemeral port)");
      return 1;
    }
    target = "127.0.0.1:" + std::to_string(ephemeral);
    const std::string listenArg = target;
    proc.pid = fork();
    if (proc.pid < 0) {
      logError("fork failed: " + std::string(std::strerror(errno)));
      return 1;
    }
    if (proc.pid == 0) {
      execl(binary.c_str(), binary.c_str(), "-listen", listenArg.c_str(),
            static_cast<char*>(nullptr));
      _exit(127);
    }
  } else {
    bool remoteOk = true;
    target = remoteTargetFromFlags(&remoteOk);
    if (!remoteOk) {
      logError("--port must be in 1..65535 when using --host");
      return 2;
    }
    if (target.empty()) {
      logError(
          "Set --target=host:port or --host and --port for the device-side "
          "evrp-device, or set --device_binary / EVRP_DEVICE_BINARY for local "
          "spawn");
      return 2;
    }
  }

  const std::shared_ptr<grpc::Channel> channel =
      evrp::device::api::makeDeviceChannel(target);

  if (!waitForPing(channel, FLAGS_rpc_wait_ms)) {
    logError("Timed out waiting for Ping on " + target);
    return 1;
  }
  logInfo("Ping ok on " + target);

  std::vector<evrp::device::api::DeviceKind> caps;
  if (!fetchCapabilities(channel, &caps)) {
    return 1;
  }

  if (!testInputListen(channel, caps)) {
    return 1;
  }
  if (!testPlayback(channel, caps)) {
    return 1;
  }

  logInfo("evrp-device integration test passed");
  return 0;
}
