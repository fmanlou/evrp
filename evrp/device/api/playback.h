#pragma once

#include "evrp/device/api/types.h"

namespace evrp::device::api {

// 与 `PlaybackService` 语义对应；gRPC/Protobuf 不得出现在此头文件中。
class IPlayback {
 public:
  virtual ~IPlayback() = default;

  // `result_out` 可为 nullptr；若非空，无论成功与否均可写入服务端返回的 code/message。
  virtual bool upload(const RecordingUpload& recording,
                      OperationResult* result_out) = 0;

  virtual bool playback(OperationResult* result_out) = 0;

  virtual bool stop_playback() = 0;
};

}  // namespace evrp::device::api
