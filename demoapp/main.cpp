#include "CrashingWidget.hpp"
#include "../crashup/Crashup.hpp"
#include "WidgetTracker.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QString>
#include <Qt>

#include <memory>

int main(int argc, char *argv[]) {
  QApplication demoapp(argc, argv);

  QCommandLineParser parser;
  parser.addHelpOption();
  QCommandLineOption track_widgets(
      "enable-widget-tracker", "Enables printing widgets status to stderr.");
  parser.addOption(track_widgets);

  parser.process(demoapp);

  std::unique_ptr<WidgetTracker> wt;
  if (parser.isSet(track_widgets)) {
    /* Enable printing coordinates and properties of all widgets so that
    hard-coding them in testing code can be avoided. */
    wt = std::unique_ptr<WidgetTracker>(new WidgetTracker(&demoapp));
  }

  /* just for the testing, generally these strings should be given by the user
   */
  std::string working_dir = ".";
  std::string server_address = "http://socorro.qed.ai:5000";

  crashup::Crashup crashup(working_dir, server_address);

  crashup.setAppName("demoapp");
  crashup.setAppVersion("0.42");

  // initialize Crashup internals
  crashup.init();

  CrashingWidget w([&](std::string event_name, std::string event_data) {},
                   [&]() { crashup.uploadPendingMinidumps(); });
  w.show();
  w.activateWindow();
  return demoapp.exec();
}
