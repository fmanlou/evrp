#include "evrp/device/client/remoteplayback.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::client {

RemotePlayback::RemotePlayback(std::shared_ptr<grpc::Channel> channel)
    : channel_(std::move(channel)),
      stub_(v1::PlaybackService::NewStub(channel_)) {}

RemotePlayback::~RemotePlayback() = default;

bool RemotePlayback::upload(const std::vector<api::InputEvent>& events,
                            api::OperationResult* result_out) {
  std::lock_guard<std::mutex> lock(callMu_);

  v1::UploadRecordingRequest req;
  api::toProto(events, req.mutable_events());

  grpc::ClientContext ctx;
  v1::OperationResult resp;
  grpc::Status st = stub_->Upload(&ctx, req, &resp);
  if (!st.ok()) {
    if (result_out) {
      result_out->code = static_cast<int32_t>(st.error_code());
      result_out->message = st.error_message();
    }
    return false;
  }
  if (result_out) {
    result_out->code = resp.code();
    result_out->message = resp.message();
  }
  return resp.code() == 0;
}

int RemotePlayback::playbackIndex() const { return reportedIndex_; }

bool RemotePlayback::playback(api::OperationResult* result_out,
                              evrp::CountingSemaphore* progress_notify) {
  std::lock_guard<std::mutex> lock(callMu_);

  reportedIndex_ = -1;

  std::unique_ptr<grpc::ClientContext> stream_ctx;
  std::unique_ptr<grpc::ClientReader<v1::PlaybackProgress>>
      reader;
  std::thread progress_thread;

  if (progress_notify != nullptr) {
    stream_ctx = std::make_unique<grpc::ClientContext>();
    google::protobuf::Empty sub_req;
    reader = stub_->SubscribePlayback(stream_ctx.get(), sub_req);

    grpc::ClientReader<v1::PlaybackProgress>* raw_reader =
        reader.get();
    progress_thread = std::thread([raw_reader, progress_notify, this]() {
      v1::PlaybackProgress msg;
      while (raw_reader->Read(&msg)) {
        reportedIndex_ = msg.event_index();
        progress_notify->release();
      }
      grpc::Status fin = raw_reader->Finish();
      (void)fin;
    });
  }

  grpc::ClientContext playback_ctx;
  v1::PlaybackRecordingRequest play_req;
  v1::OperationResult pb_result;
  grpc::Status st = stub_->Playback(&playback_ctx, play_req, &pb_result);

  if (progress_thread.joinable()) {
    progress_thread.join();
  }

  if (result_out) {
    if (st.ok()) {
      result_out->code = pb_result.code();
      result_out->message = pb_result.message();
    } else {
      result_out->code = static_cast<int32_t>(st.error_code());
      result_out->message = st.error_message();
    }
  }
  return st.ok() && pb_result.code() == 0;
}

bool RemotePlayback::stopPlayback() {
  std::lock_guard<std::mutex> lock(callMu_);

  grpc::ClientContext ctx;
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  return stub_->Stop(&ctx, req, &resp).ok();
}

}  // namespace evrp::device::client
