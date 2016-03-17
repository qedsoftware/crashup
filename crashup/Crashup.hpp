#include <string>
#include "updates/Updates.hpp"
#include "stats/Stats.hpp"
#include "SettingsWidget.hpp"

namespace crashup {

class Crashup {
  std::string working_dir, server_address;

public:
  Crashup(std::string working_dir, std::string server_address);
  Updates &updates();
  Stats &stats();
  SettingsWidget &createSettingsWidget();
  std::string getFileRevisions();        // for stats and crash handler
  void setPollingInterval(int interval); // for SettingsWidget
};
}; // namespace crashup
