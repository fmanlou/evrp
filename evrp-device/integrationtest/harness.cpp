#include "harness.h"

#include <gflags/gflags.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <thread>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "evrp/device/internal/discovery/devicediscovery.h"
#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

DECLARE_int32(discovery_port);
DECLARE_string(discovery_link_mode);

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
    "Maximum time to wait for GetCapabilities after connect attempts begin (ms)");
DEFINE_bool(test_input_listen, true,
            "Exercise InputListenService (StartRecording / Wait / Read / Stop)");
DEFINE_bool(test_playback, true,
            "Exercise PlaybackService when device reports keyboard capability");
DEFINE_int32(listen_wait_ms, 400,
             "WaitForInputEvent timeout (ms) per poll while testing "
             "InputListen; must be >= 0");
DEFINE_int32(
    listen_per_kind_timeout_ms, 120000,
    "When --listen_require_valid_event_per_kind=true: max wall time (ms) per "
    "supported kind to receive at least one non-EV_SYN event");
DEFINE_int32(
    listen_between_kinds_ms, 80,
    "After StopRecording, wait this many ms before the next StartRecording "
    "(strict per-kind mode; helps server drain session)");
DEFINE_bool(
    listen_require_valid_event_per_kind, true,
    "If true, InputListen requires a non-SYN evdev event per supported kind "
    "(sequential sessions); if false, one short listen round without requiring "
    "events (CI / no hardware)");
DEFINE_bool(
    test_udp_discovery, true,
    "With --device_binary / EVRP_DEVICE_BINARY: verify makeClient(\"\") finds "
    "the spawned evrp-device via UDP (--discovery_port must match between "
    "processes; skipped for remote --target/--host).");
DEFINE_string(
    log_level, "info",
    "This process log level: error|warn|info|debug|trace|off; with "
    "--device_binary, also passed to the spawned evrp-device");

namespace {

MemorySetting harnessDiscoverySettingsSnapshot() {
  MemorySetting s;
  s.insert(evrp::sdk::kDeviceDiscoverySettingPort, FLAGS_discovery_port);
  s.insert(evrp::sdk::kDeviceDiscoverySettingLinkMode, FLAGS_discovery_link_mode);
  return s;
}

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

int pickFreeUdpPort() {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
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

std::unique_ptr<DeviceProcess> g_proc;

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

std::vector<evrp::device::api::DeviceKind> dedupeKindsPreserveOrder(
    const std::vector<evrp::device::api::DeviceKind>& kinds) {
  std::vector<evrp::device::api::DeviceKind> out;
  for (evrp::device::api::DeviceKind k : kinds) {
    if (k == evrp::device::api::DeviceKind::kUnspecified) {
      continue;
    }
    if (std::find(out.begin(), out.end(), k) == out.end()) {
      out.push_back(k);
    }
  }
  return out;
}

bool isValidListenProbeEvent(
    evrp::device::api::DeviceKind expectedKind,
    const evrp::device::api::InputEvent& e) {
  if (e.device != expectedKind) {
    return false;
  }
  if (e.type == static_cast<uint32_t>(EV_SYN)) {
    return false;
  }
  return true;
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

}  // namespace

IntegrationEnv IntegrationHarness::env_{};
bool IntegrationHarness::initialized_{false};

bool IntegrationHarness::initialize() {
  if (initialized_) {
    return true;
  }
  env_ = IntegrationEnv{};

  const std::string binary = deviceBinaryPath();
  if (!binary.empty()) {
    if (!isExecutableFile(binary)) {
      logError("Not an executable file: {}", binary);
      return false;
    }
    env_.discovery_udp_port = pickFreeUdpPort();
    if (env_.discovery_udp_port <= 0) {
      logError("pickFreeUdpPort failed (bind loopback ephemeral UDP port)");
      return false;
    }
    const int ephemeral = pickFreeLoopbackPort();
    if (ephemeral <= 0) {
      logError("pickFreeLoopbackPort failed (bind loopback ephemeral port)");
      return false;
    }
    env_.target = "127.0.0.1:" + std::to_string(ephemeral);
    env_.spawned_local = true;
    const std::string listenArg = env_.target;
    const std::string discoveryFlag =
        std::string("--discovery_port=") +
        std::to_string(env_.discovery_udp_port);
    const std::string linkModeFlag =
        std::string("--discovery_link_mode=") + FLAGS_discovery_link_mode;
    g_proc = std::make_unique<DeviceProcess>();
    g_proc->pid = fork();
    if (g_proc->pid < 0) {
      logError("fork failed: {}", std::strerror(errno));
      g_proc.reset();
      return false;
    }
    if (g_proc->pid == 0) {
      const std::string logFlag =
          std::string("--log_level=") + FLAGS_log_level;
      execl(binary.c_str(), binary.c_str(), "-listen", listenArg.c_str(),
            discoveryFlag.c_str(), linkModeFlag.c_str(), logFlag.c_str(),
            static_cast<char*>(nullptr));
      _exit(127);
    }
  } else {
    bool remoteOk = true;
    env_.target = remoteTargetFromFlags(&remoteOk);
    if (!remoteOk) {
      logError("--port must be in 1..65535 when using --host");
      return false;
    }
    if (env_.target.empty()) {
      if (FLAGS_discovery_port <= 0) {
        logError(
            "Without --target/--host, set --discovery_port > 0 to match "
            "evrp-device (or use --device_binary / --target for full suite)");
        return false;
      }
      logInfo(
          "No --target/--host: discovery-only mode; non-discovery tests skip "
          "unless you add direct endpoint flags");
    }
    env_.discovery_udp_port = FLAGS_discovery_port;
  }

  initialized_ = true;
  return true;
}

void IntegrationHarness::shutdown() {
  g_proc.reset();
  env_ = IntegrationEnv{};
  initialized_ = false;
}

const IntegrationEnv& IntegrationHarness::env() { return env_; }

bool IntegrationHarness::hasDirectTarget() { return !env_.target.empty(); }

std::unique_ptr<evrp::device::api::IClient>
IntegrationHarness::connectDirectClient(int timeout_ms) {
  const auto connectDeadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds(timeout_ms);
  while (std::chrono::steady_clock::now() < connectDeadline) {
    auto client = evrp::device::api::makeClient(env_.target,
                                                harnessDiscoverySettingsSnapshot());
    if (client) {
      return client;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
  logError("Timed out waiting for SessionService/Connect on {}", env_.target);
  return nullptr;
}

bool IntegrationHarness::waitUntilGetCapabilitiesOk(
    evrp::device::api::IClient& client, int total_timeout_ms) {
  evrp::device::api::IInputDeviceClient* const device = client.inputDevice();
  if (!device) {
    return false;
  }
  const auto overall =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds(total_timeout_ms);
  while (std::chrono::steady_clock::now() < overall) {
    std::vector<evrp::device::api::DeviceKind> kinds;
    if (device->getCapabilities(&kinds)) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
  logError("Timed out waiting for GetCapabilities on {}", env_.target);
  return false;
}

bool IntegrationHarness::fetchCapabilities(
    evrp::device::api::IClient& client,
    std::vector<evrp::device::api::DeviceKind>* kinds_out) {
  kinds_out->clear();
  evrp::device::api::IInputDeviceClient* const device = client.inputDevice();
  if (!device) {
    logError("GetCapabilities failed (no input device client)");
    return false;
  }
  if (!device->getCapabilities(kinds_out)) {
    logError("GetCapabilities failed");
    return false;
  }
  logInfo("GetCapabilities ok ({} kind(s))", kinds_out->size());
  return true;
}

bool IntegrationHarness::runInputListenTest(
    evrp::device::api::IClient& client,
    const std::vector<evrp::device::api::DeviceKind>& caps) {
  if (!FLAGS_test_input_listen) {
    logInfo("InputListen: skipped (--test_input_listen=false)");
    return true;
  }
  std::vector<evrp::device::api::DeviceKind> kinds =
      dedupeKindsPreserveOrder(kindsForRecording(caps));
  if (kinds.empty()) {
    logInfo(
        "InputListen: skip (no input device kinds on device; recording N/A)");
    return true;
  }
  if (FLAGS_listen_wait_ms < 0) {
    logError("--listen_wait_ms must be >= 0");
    return false;
  }

  evrp::device::api::IInputListener* const listener = client.inputListener();
  if (!listener) {
    logError("InputListen: no input listener");
    return false;
  }

  if (!FLAGS_listen_require_valid_event_per_kind) {
    if (!listener->startListening(kinds)) {
      logError(
          "InputListen: StartRecording failed (no matching devices or error)");
      return false;
    }
    (void)listener->waitForInputEvent(FLAGS_listen_wait_ms);
    const std::vector<evrp::device::api::InputEvent> batch =
        listener->readInputEvents();
    logInfo(
        "InputListen: ReadInputEvents count={} "
        "(--listen_require_valid_event_per_kind=false)",
        batch.size());
    listener->cancelListening();
    logInfo("InputListen: StopRecording ok");
    return true;
  }

  if (FLAGS_listen_per_kind_timeout_ms < 1) {
    logError(
        "--listen_per_kind_timeout_ms must be >= 1 when "
        "--listen_require_valid_event_per_kind=true");
    return false;
  }

  for (size_t ki = 0; ki < kinds.size(); ++ki) {
    const evrp::device::api::DeviceKind kind = kinds[ki];
    const std::string kindLabel = evrp::device::api::deviceKindLabel(kind);
    if (ki > 0 && FLAGS_listen_between_kinds_ms > 0) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(FLAGS_listen_between_kinds_ms));
    }
    if (!listener->startListening({kind})) {
      logError(
          "InputListen: StartRecording failed for kind {} (client log shows "
          "gRPC error; evrp-device log shows LocalInputListener path/open "
          "details)",
          kindLabel);
      return false;
    }
    logInfo(
        "InputListen: waiting for a non-EV_SYN event on kind {} ({} ms per "
        "kind; operate that device or wait for timeout)",
        kindLabel,
        FLAGS_listen_per_kind_timeout_ms);
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(
                              FLAGS_listen_per_kind_timeout_ms);
    bool gotValid = false;
    while (std::chrono::steady_clock::now() < deadline && !gotValid) {
      const auto remaining =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              deadline - std::chrono::steady_clock::now());
      int slice = FLAGS_listen_wait_ms;
      if (remaining.count() < slice) {
        slice = static_cast<int>(std::max<int64_t>(remaining.count(), 0));
      }
      (void)listener->waitForInputEvent(slice);
      const std::vector<evrp::device::api::InputEvent> batch =
          listener->readInputEvents();
      for (const evrp::device::api::InputEvent& e : batch) {
        if (isValidListenProbeEvent(kind, e)) {
          gotValid = true;
          logInfo(
              "InputListen: valid event on {} type={:#x} code={:#x} value={}",
              kindLabel,
              e.type,
              e.code,
              e.value);
          break;
        }
      }
    }
    listener->cancelListening();
    if (!gotValid) {
      logError(
          "InputListen: timeout waiting for non-EV_SYN event on kind {} within "
          "{} ms (operate that device or use "
          "--listen_require_valid_event_per_kind=false for CI)",
          kindLabel,
          FLAGS_listen_per_kind_timeout_ms);
      return false;
    }
  }
  logInfo(
      "InputListen: all {} kind(s) produced at least one valid event; "
      "StopRecording ok",
      kinds.size());
  return true;
}

bool IntegrationHarness::runPlaybackTest(
    evrp::device::api::IClient& client,
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
  evrp::device::api::IPlayback* const playback = client.playback();
  if (!playback) {
    logError("Playback: no playback client");
    return false;
  }
  evrp::device::api::OperationResult up;
  if (!playback->upload(events, &up)) {
    logError("Playback: Upload failed code={} msg={}", up.code, up.message);
    return false;
  }
  logInfo("Playback: Upload ok ({} events)", events.size());

  evrp::device::api::OperationResult play;
  const bool ok = playback->playback(&play, nullptr);
  if (!ok) {
    logError(
        "Playback: Playback failed code={} msg={} (evrp-device must be able to "
        "open a keyboard evdev for O_RDWR and inject KEY events; try sudo or "
        "a device with injectable /dev/input/event*)",
        play.code,
        play.message);
    return false;
  }
  logInfo("Playback: Playback RPC finished ok");
  return true;
}

bool IntegrationHarness::runUdpDiscoveryTest() {
  if (env_.discovery_udp_port <= 0 || !FLAGS_test_udp_discovery) {
    if (env_.spawned_local && !FLAGS_test_udp_discovery) {
      logInfo("UDP discovery: skipped (--test_udp_discovery=false)");
    }
    return true;
  }
  if (!evrp::device::api::useUdpDeviceDiscovery("")) {
    logError("Internal: useUdpDeviceDiscovery(\"\") expected true");
    return false;
  }
  const int saved_discovery = FLAGS_discovery_port;
  FLAGS_discovery_port = env_.discovery_udp_port;
  std::unique_ptr<evrp::device::api::IClient> viaDiscovery;
  {
    const auto connectDeadline =
        std::chrono::steady_clock::now() +
        std::chrono::milliseconds(FLAGS_rpc_wait_ms);
    while (std::chrono::steady_clock::now() < connectDeadline) {
      viaDiscovery = evrp::device::api::makeClient(
          "", harnessDiscoverySettingsSnapshot());
      if (viaDiscovery) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
  }
  FLAGS_discovery_port = saved_discovery;
  if (!viaDiscovery) {
    logError(
        "UDP discovery: timed out makeClient(\"\") with --discovery_port={}. "
        "Start `evrp-device` on the LAN with the same --discovery_port (default "
        "53508). Discovery-only from this host cannot reach a container in "
        "bridge networking; use Docker --network=host or pass --target=HOST:PORT.",
        env_.discovery_udp_port);
    return false;
  }
  const std::string& addr = viaDiscovery->serverAddress();
  if (addr.empty()) {
    logError("UDP discovery: connected client has empty serverAddress");
    return false;
  }
  if (!env_.target.empty() && addr != env_.target) {
    logError(
        "UDP discovery: serverAddress {} != direct target {}",
        addr,
        env_.target);
    return false;
  }
  logInfo("UDP discovery: makeClient(\"\") ok, serverAddress={}", addr);
  return true;
}
