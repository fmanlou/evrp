#include "evrp/device/client/remoteplayback.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/device/common/devicesessionmetadata.h"
#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::client {

RemotePlayback::RemotePlayback(std::shared_ptr<grpc::Channel> channel,
                               std::string deviceSessionId)
    : channel_(std::move(channel)),
      stub_(v1::PlaybackService::NewStub(channel_)),
      deviceSessionId_(std::move(deviceSessionId)) {}

RemotePlayback::~RemotePlayback() = default;

bool RemotePlayback::upload(const std::vector<api::InputEvent>& events,
                            api::OperationResult* resultOut) {
  std::lock_guard<std::mutex> lock(callMu_);

  v1::UploadRecordingRequest req;
  api::toProto(events, req.mutable_events());

  grpc::ClientContext ctx;
  addDeviceSessionMetadata(&ctx, deviceSessionId_);
  v1::OperationResult resp;
  grpc::Status st = stub_->Upload(&ctx, req, &resp);
  if (!st.ok()) {
    if (resultOut) {
      resultOut->code = static_cast<int32_t>(st.error_code());
      resultOut->message = st.error_message();
    }
    return false;
  }
  if (resultOut) {
    resultOut->code = resp.code();
    resultOut->message = resp.message();
  }
  return resp.code() == 0;
}

int RemotePlayback::playbackIndex() const { return reportedIndex_; }

bool RemotePlayback::playback(api::OperationResult* resultOut,
                              evrp::CountingSemaphore* progressNotify) {
  std::lock_guard<std::mutex> lock(callMu_);

  reportedIndex_ = -1;

  std::unique_ptr<grpc::ClientContext> stream_ctx;
  std::unique_ptr<grpc::ClientReader<v1::PlaybackProgress>>
      reader;
  std::thread progress_thread;

  if (progressNotify != nullptr) {
    stream_ctx = std::make_unique<grpc::ClientContext>();
    addDeviceSessionMetadata(stream_ctx.get(), deviceSessionId_);
    google::protobuf::Empty sub_req;
    reader = stub_->SubscribePlayback(stream_ctx.get(), sub_req);

    grpc::ClientReader<v1::PlaybackProgress>* raw_reader =
        reader.get();
    progress_thread = std::thread([raw_reader, progressNotify, this]() {
      v1::PlaybackProgress msg;
      while (raw_reader->Read(&msg)) {
        reportedIndex_ = msg.event_index();
        progressNotify->release();
      }
      grpc::Status fin = raw_reader->Finish();
      (void)fin;
    });
  }

  grpc::ClientContext playback_ctx;
  addDeviceSessionMetadata(&playback_ctx, deviceSessionId_);
  v1::PlaybackRecordingRequest play_req;
  v1::OperationResult pb_result;
  grpc::Status st = stub_->Playback(&playback_ctx, play_req, &pb_result);

  if (progress_thread.joinable()) {
    progress_thread.join();
  }

  if (resultOut) {
    if (st.ok()) {
      resultOut->code = pb_result.code();
      resultOut->message = pb_result.message();
    } else {
      resultOut->code = static_cast<int32_t>(st.error_code());
      resultOut->message = st.error_message();
    }
  }
  return st.ok() && pb_result.code() == 0;
}

bool RemotePlayback::stopPlayback() {
  std::lock_guard<std::mutex> lock(callMu_);

  grpc::ClientContext ctx;
  addDeviceSessionMetadata(&ctx, deviceSessionId_);
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  return stub_->Stop(&ctx, req, &resp).ok();
}

}
