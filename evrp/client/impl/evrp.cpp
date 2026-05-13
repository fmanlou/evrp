#include "evrp/client/api/evrp.h"

namespace evrp::client {

int runnerRecord(std::shared_ptr<ISetting> settings);
int runnerReplay(std::shared_ptr<ISetting> settings);

int record(std::shared_ptr<ISetting> settings) {
  return runnerRecord(std::move(settings));
}

int replay(std::shared_ptr<ISetting> settings) {
  return runnerReplay(std::move(settings));
}

}  // namespace evrp::client
