#pragma once

#include "evrp/sdk/syncdispatchqueue.h"

namespace evrp::device::server {

class IoContextPostedBase {
 public:
  explicit IoContextPostedBase(asio::io_context& ioContext);
  IoContextPostedBase(const IoContextPostedBase&) = delete;
  IoContextPostedBase& operator=(const IoContextPostedBase&) = delete;
  virtual ~IoContextPostedBase() = default;

 protected:
  void shutdown();

 protected:
  SyncDispatchQueue syncDispatch_;
};

}  // namespace evrp::device::server
