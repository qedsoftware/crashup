#include "CrashingWidget.hpp"
#include "../crashup/Crashup.hpp"
#include "WidgetTracker.hpp"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
  QApplication demoapp(argc, argv);
  qDebug() << "QT version: " << QT_VERSION_STR;
  /* Enable printing coordinates and properties of all widgets so that
  hard-coding them in testing code can be avoided. This is only for testing
  purposes and should be turned off on release builds. */
  WidgetTracker tracker;
  demoapp.installEventFilter(&tracker);

  /* just for the testing, generally these strings should be given my the user
   */
  std::string working_dir_path = ".";
  std::string server_address = "servers_address";
  std::string report_minidumps_relative_dirpath =
      "minidumps"; // relative to the working_dir_path ! be careful not to start
                   // with "/"

  crashup::Crashup crashup(working_dir_path, server_address);

  /* get the breakpad handler going -- minidumps written to a requested dir */
  /* throws exception if requested path is inaccessible */
  crashup.initCrashHandler(report_minidumps_relative_dirpath);

  // crashup.writeMinidump();	/* creates and saves Minidump of the current
  // state on demand (does what would happen in case of a crash, without any
  // actual crash) */
  /* useful for testing or for unusual uses -- executions continues after that !
   */
  CrashingWidget w([&](std::string event_name, std::string event_data) {
    crashup.stats().logEvent(event_name, event_data);
  });
  w.show();

  return demoapp.exec();
}
