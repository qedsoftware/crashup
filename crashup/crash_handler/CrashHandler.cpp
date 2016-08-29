#include <QtGlobal>

#if defined(Q_OS_LINUX)

#include "CrashHandler.hpp"
#include <QtCore/QProcess>

#include <stdio.h>

#include "client/linux/handler/exception_handler.h"

namespace crash_handling {

/************************************************************************/
/* CrashHandlerPrivate                                                  */
/************************************************************************/
class CrashHandlerPrivate {
public:
  static google_breakpad::ExceptionHandler *pHandler;
  static bool bReportCrashesToSystem;

  CrashHandlerPrivate() {}
  ~CrashHandlerPrivate() {}

  void initCrashHandlerPrivate(const std::string &report_minidumps_dirpath);
};

/***********/
/* statics */
/***********/
google_breakpad::ExceptionHandler *CrashHandlerPrivate::pHandler = nullptr;
bool CrashHandlerPrivate::bReportCrashesToSystem = false;

/*****************************************/
/* dumpCallback Function -- OS dependant */
/*****************************************/

bool dumpCallback(const google_breakpad::MinidumpDescriptor &md, void *context,
                  bool success) {
  Q_UNUSED(context);
  qDebug("BreakpadQt crash");

  // printing to stdout makes the string is printed nither to stdout nor stderr
  // this made hard to read minidump from program output
  fprintf(stderr, "Minidump saved as: %s\n", md.path());

  /*
  NO STACK USE, NO HEAP USE THERE !!!
  Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
  */
  return CrashHandlerPrivate::bReportCrashesToSystem ? success : true;
}

void CrashHandlerPrivate::initCrashHandlerPrivate(
    const std::string &report_minidumps_dirpath) {

  if (pHandler != nullptr) {
    qWarning("The Google Breakpad ExceptionHandler is already initialized.");
    return;
  }

  google_breakpad::MinidumpDescriptor md(report_minidumps_dirpath);
  pHandler = new google_breakpad::ExceptionHandler(
      md,           /* MinidumpDescriptor */
      nullptr,      /* FilterCallback, run before writing minidump to file */
      dumpCallback, /* DumpCallback function, launched after writing minidump to
                       file
                                            should it return true => exception
                       is treated as handled
                                            else => Breakpad treats exception as
                       unhandled and allows another handler to handle it
                                    */
      nullptr,      /* (void*) callback_context */
      true,         /* install handler?
                                    true => minidump will be written whenever
                                    an unhandled exception occurs
                                    false => only when writeMinidump() is called
                            */
      -1            /* const int server_fd
                            -1 => in-process minidump generation will
                    */
      );
}

/************************************************************************/
/* CrashHandler                                                         */
/************************************************************************/

CrashHandler::CrashHandler() { pPrivCrashHandler = new CrashHandlerPrivate(); }

CrashHandler::~CrashHandler() { delete pPrivCrashHandler; }

CrashHandler *CrashHandler::instance() {
  static CrashHandler globalHandler;
  return &globalHandler;
}

void CrashHandler::init(const std::string &report_minidumps_dirpath) {
  pPrivCrashHandler->initCrashHandlerPrivate(report_minidumps_dirpath);
}

void CrashHandler::setReportCrashesToSystem(bool toReport) {
  pPrivCrashHandler->bReportCrashesToSystem = toReport;
}

bool CrashHandler::writeMinidump() {
  bool res = pPrivCrashHandler->pHandler->WriteMinidump();
  if (res) {
    qDebug("BreakpadQt: writeMinidump() successed.");
  } else {
    qWarning("BreakpadQt: writeMinidump() failed.");
  }
  return res;
}
}

#endif
