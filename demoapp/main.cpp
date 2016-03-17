#include "CrashingWidget.hpp"
#include "../crashup/Crashup.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication demoapp(argc, argv);

    crashup::Crashup crashup("working_dir", "server_address");
    CrashingWidget w([&](std::string event_name, std::string event_data){crashup.stats().logEvent(event_name, event_data);});
    w.show();

    return demoapp.exec();
}
