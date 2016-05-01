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
  std::string working_dir_path = ".";
  std::string server_address =
      "http://ec2-52-91-29-60.compute-1.amazonaws.com/submit";

  crashup::Crashup crashup(working_dir_path, server_address);

  /* get the breakpad handler going -- minidumps written to a requested dir */
  /* throws exception if requested path is inaccessible */
  crashup.initCrashHandler();

  std::string product_name = "Test";
  std::string product_version = "1.0";

  /* initiates CrashUploader, configured to upload onto server_address */
  /* given to the crashup constructor */
  crashup.initCrashUploader(product_name, product_version);

  // crashup.writeMinidump();	/* creates and saves Minidump of the current
  // state on demand (does what would happen in case of a crash, without any
  // actual crash) */
  /* useful for testing or for unusual uses -- executions continues after that !
   */

  CrashingWidget w(
      [&](std::string event_name, std::string event_data) {
        crashup.stats().logEvent(event_name, event_data);
      },
      [&]() { crashup.uploadPendingMinidumps(); });
  w.show();
  w.activateWindow();
  return demoapp.exec();
}
