#pragma once

#include <memory>

#include "evrp/server/api/evrp.h"
#include "evrp/sdk/iocontextpostedbase.h"

namespace evrp::server {

class PostedEvrp final : public Evrp,
                         private evrp::device::server::IoContextPostedBase {
 public:
  using IoContextPostedBase::shutdown;

  PostedEvrp(Evrp& inner, asio::io_context& ioContext);
  ~PostedEvrp() override;

  PostedEvrp(const PostedEvrp&) = delete;
  PostedEvrp& operator=(const PostedEvrp&) = delete;

  int record(std::shared_ptr<ISetting> settings) override;
  int replay(std::shared_ptr<ISetting> settings) override;

 private:
  Evrp& inner_;
};

}  // namespace evrp::server
