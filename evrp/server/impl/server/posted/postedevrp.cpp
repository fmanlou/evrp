#include "evrp/server/impl/server/posted/postedevrp.h"

#include <asio/io_context.hpp>

namespace evrp::server {

PostedEvrp::PostedEvrp(Evrp& inner, asio::io_context& ioContext)
    : IoContextPostedBase(ioContext), inner_(inner) {}

PostedEvrp::~PostedEvrp() { shutdown(); }

int PostedEvrp::record(std::shared_ptr<ISetting> settings) {
  return syncDispatch_.postSync<int>(
      [this, settings = std::move(settings)]() mutable {
        return inner_.record(std::move(settings));
      });
}

int PostedEvrp::replay(std::shared_ptr<ISetting> settings) {
  return syncDispatch_.postSync<int>(
      [this, settings = std::move(settings)]() mutable {
        return inner_.replay(std::move(settings));
      });
}

}  // namespace evrp::server
