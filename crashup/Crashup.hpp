#include <string>
#include "updates/Updates.hpp"
#include "stats/Stats.hpp"
#include "SettingsWidget.hpp"
#include "crash_handler/CrashHandler.hpp"
#include <QtCore/QProcess>


namespace crashup {
  
class Crashup {

private:
  std::string working_dir, server_address;
  Stats _stats;
  crashhandler::CrashHandler * _crashHandler;

public:
  Crashup(std::string working_dir, std::string server_address);
  Updates &updates();
  Stats &stats();

  /* initiates the _crashHandler */
  void initCrashHandler(const std::string& report_minidumps_path);	
  /* writes a minidump whenever asked for */
  void writeMinidump();

  SettingsWidget &createSettingsWidget();
  std::string getFileRevisions();        // for stats and crash handler
  void setPollingInterval(int interval); // for SettingsWidget
};

}; // namespace crashup
