#pragma once

#include <memory>

#include "evrp/sdk/setting/isetting.h"

namespace evrp::server {

int record(std::shared_ptr<ISetting> settings);

int replay(std::shared_ptr<ISetting> settings);

}

