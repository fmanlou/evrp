#include "evrp/device/server/posted/iocontextpostedbase.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

IoContextPostedBase::IoContextPostedBase(asio::io_context& ioContext)
    : syncDispatch_(ioContext) {}

void IoContextPostedBase::shutdown() {
  syncDispatch_.shutdown();
}

}  // namespace evrp::device::server
