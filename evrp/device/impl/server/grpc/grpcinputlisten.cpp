#include "evrp/device/impl/server/grpc/grpcinputlisten.h"

#include "evrp/device/internal/tofromproto.h"
#include "evrp/sdk/sessioncheck.h"
#include "evrp/sdk/sessionregistry.h"
#include "evrp/sdk/ioc.h"
#include "logger.h"

#include <gflags/gflags.h>
#include <google/protobuf/empty.pb.h>

#include <algorithm>
#include <chrono>
#include <vector>

DEFINE_int32(
    recording_idle_timeout_ms,
    0,
    "If >0, cancel recording after this many milliseconds without "
    "StartRecording/WaitForInputEvent/ReadInputEvents activity. 0 disables.");

namespace evrp::device::server {

namespace {

int64_t steadyNowNs() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

}  

GrpcInputListenService::GrpcInputListenService(
    const evrp::Ioc& ioc, evrp::session::SessionRegistry& sessions)
    : listener_(ioc.get<api::IInputListener>()), sessions_(sessions) {
  lastRecordingActivityNs_.store(steadyNowNs(), std::memory_order_relaxed);
  watchdogThread_ = std::thread([this] { watchdogLoop(); });
}

GrpcInputListenService::~GrpcInputListenService() {
  watchdogStop_.store(true, std::memory_order_release);
  if (watchdogThread_.joinable()) {
    watchdogThread_.join();
  }
}

void GrpcInputListenService::recordingActivityBump() {
  lastRecordingActivityNs_.store(steadyNowNs(), std::memory_order_relaxed);
}

void GrpcInputListenService::watchdogLoop() {
  while (!watchdogStop_.load(std::memory_order_acquire)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    const int timeout_ms = FLAGS_recording_idle_timeout_ms;
    if (timeout_ms <= 0 || !listener_ || !listener_->isListening()) {
      continue;
    }
    const int64_t last =
        lastRecordingActivityNs_.load(std::memory_order_relaxed);
    const int64_t idle_ns = steadyNowNs() - last;
    if (idle_ns > static_cast<int64_t>(timeout_ms) * 1'000'000) {
      logWarn(
          "evrp-device: recording idle timeout ({} ms); stopping recording "
          "(no recent InputListen RPC activity)",
          timeout_ms);
      listener_->cancelListening();
    }
  }
}

grpc::Status GrpcInputListenService::StartRecording(
    grpc::ServerContext* context,
    const v1::StartRecordingRequest* request,
    google::protobuf::Empty* ) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  if (listener_->isListening()) {
    logWarn(
        "evrp-device: StartRecording: stopping previous session before "
        "starting a new one");
    listener_->cancelListening();
  }
  if (listener_->isListening()) {
    logError(
        "evrp-device: StartRecording: session still active after "
        "cancelListening");
    return grpc::Status(
        grpc::StatusCode::ABORTED,
        "could not clear previous recording session; retry StopRecording");
  }
  std::vector<api::DeviceKind> kinds;
  api::fromProto(request->kinds(), &kinds);
  if (!listener_->startListening(kinds)) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        "startListening failed: no evdev opened for requested kinds (see "
        "evrp-device log: LocalInputListener lines)");
  }
  recordingActivityBump();
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::WaitForInputEvent(
    grpc::ServerContext* context,
    const v1::WaitForInputEventRequest* request,
    v1::WaitForInputEventResponse* response) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  if (request->timeout_ms() < 0) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "timeout_ms must be >= 0");
  }
  if (!listener_->isListening()) {
    response->set_ready(false);
    return grpc::Status::OK;
  }
  recordingActivityBump();
  const int timeout_ms = request->timeout_ms();
  if (timeout_ms == 0) {
    if (context->IsCancelled()) {
      logWarn(
          "evrp-device: WaitForInputEvent: client disconnected; stopping "
          "recording");
      listener_->cancelListening();
      response->set_ready(false);
      return grpc::Status::OK;
    }
    response->set_ready(listener_->waitForInputEvent(0));
    return grpc::Status::OK;
  }

  constexpr int kDisconnectPollSliceMs = 150;
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(timeout_ms);

  while (listener_->isListening()) {
    recordingActivityBump();
    if (context->IsCancelled()) {
      logWarn(
          "evrp-device: WaitForInputEvent: client disconnected; stopping "
          "recording");
      listener_->cancelListening();
      response->set_ready(false);
      return grpc::Status::OK;
    }
    const auto now = std::chrono::steady_clock::now();
    if (now >= deadline) {
      response->set_ready(false);
      return grpc::Status::OK;
    }
    const int remaining_ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now)
            .count());
    const int slice_ms =
        std::min(kDisconnectPollSliceMs, std::max(1, remaining_ms));
    if (listener_->waitForInputEvent(slice_ms)) {
      response->set_ready(true);
      return grpc::Status::OK;
    }
  }
  response->set_ready(false);
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::ReadInputEvents(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    v1::ReadInputEventsResponse* response) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  if (context->IsCancelled()) {
    logWarn(
        "evrp-device: ReadInputEvents: client disconnected; stopping recording");
    listener_->cancelListening();
  }
  if (listener_->isListening()) {
    recordingActivityBump();
  }
  std::vector<api::InputEvent> batch = listener_->readInputEvents();
  for (const api::InputEvent& e : batch) {
    api::toProto(e, response->add_events());
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::StopRecording(
    grpc::ServerContext* context, const google::protobuf::Empty* ,
    google::protobuf::Empty* ) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  if (listener_->isListening()) {
    listener_->cancelListening();
  }
  return grpc::Status::OK;
}

}
