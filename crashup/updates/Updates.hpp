#include <string>

class Updates {
  std::string working_dir, server_address;

public:
  Updates(std::string working_dir, std::string server_address);
  void registerFile(std::string local_path, std::string remote_name);
  void registerDirectory(std::string local_path, std::string remote_name);
  void setPollingInterval(int interval);
  std::string getFileRevisions();
};
