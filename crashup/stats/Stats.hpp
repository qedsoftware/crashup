#include <string>

class Stats {
  std::string working_dir, server_address;

public:
  Stats(std::string working_dir, std::string server_address);
  void logEvent(std::string event_name, std::string event_data);
};
