#include "CrashingWidget.hpp"
#include "../crashup/Crashup.hpp"
#include "WidgetTracker.hpp"
#include <QApplication>
#include <QDebug>
#include <QString>
#include <Qt>

int main(int argc, char *argv[]) {
  QApplication demoapp(argc, argv);
  /* Enable printing coordinates and properties of all widgets so that
  hard-coding them in testing code can be avoided. This is only for testing
  purposes and should be turned off on release builds. */
  WidgetTracker wt(&demoapp);

  /* just for the testing, generally these strings should be given my the user
   */
  std::string working_dir = ".";
  std::string server_address = "http://ec2-52-91-29-60.compute-1.amazonaws.com";

  crashup::Crashup crashup(working_dir, server_address);

  crashup.setAppName("demoapp");
  crashup.setAppVersion("0.42");
#if defined(Q_OS_WIN32)
  crashup.setAppPlatform("windows");
#else
  crashup.setAppPlatform("linux");
#endif
  /* get the breakpad / crashpad handler going -- minidumps written to a
  requested dir throws exception if requested path is inaccessible */
  crashup.initCrashHandler();

  /* initiates CrashUploader, configured to upload onto server_address */
  /* given to the crashup constructor */
  crashup.initCrashUploader();

  CrashingWidget w(
      [&](std::string event_name, std::string event_data) {
        crashup.sendUsageReport(event_name, event_data);
      },
      [&]() { crashup.uploadPendingMinidumps(); });
  w.show();
  w.activateWindow();
  return demoapp.exec();
}
