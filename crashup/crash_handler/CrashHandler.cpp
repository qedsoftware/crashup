#include "CrashHandler.hpp"
#include <QtCore/QProcess>

namespace crash_handling {

/************************************************************************/
/* CrashHandlerPrivate                                                  */
/************************************************************************/
class CrashHandlerPrivate {
public:
  static google_breakpad::ExceptionHandler *pHandler;
  static bool bReportCrashesToSystem;

  CrashHandlerPrivate() { pHandler = NULL; }
  ~CrashHandlerPrivate() { delete pHandler; }

  void initCrashHandlerPrivate(const std::string &report_minidumps_dirpath);
};

/***********/
/* statics */
/***********/
google_breakpad::ExceptionHandler *CrashHandlerPrivate::pHandler = nullptr;
bool CrashHandlerPrivate::bReportCrashesToSystem = false;

/*****************************************/
/* cumpCallback Function -- OS dependant */
/*****************************************/

#if defined(Q_OS_WIN32)
bool dumpCallback(const wchar_t *_dump_dir, const wchar_t *_minidump_id,
                  void *context, EXCEPTION_POINTERS *exinfo,
                  MDRawAssertionInfo *assertion, bool success)
#elif defined(Q_OS_LINUX)
bool dumpCallback(const google_breakpad::MinidumpDescriptor &md, void *context,
                  bool success)
#elif defined(Q_OS_MAC)
bool dumpCallback(const char *_dump_dir, const char *_minidump_id,
                  void *context, bool success)
#endif
{
  Q_UNUSED(context);
#if defined(Q_OS_WIN32)
  Q_UNUSED(_dump_dir);
  Q_UNUSED(_minidump_id);
  Q_UNUSED(assertion);
  Q_UNUSED(exinfo);
#endif
  qDebug("BreakpadQt crash");

#if defined(Q_OS_LINUX)
  printf("Minidump saved as: %s\n", md.path());
#elif defined(Q_OS_WIN32)
  printf("Minidump saved as: %ls\n", _dump_dir);
#endif

  /*
  NO STACK USE, NO HEAP USE THERE !!!
  Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
  */
  return CrashHandlerPrivate::bReportCrashesToSystem ? success : true;
}

void CrashHandlerPrivate::initCrashHandlerPrivate(
    const std::string &report_minidumps_dirpath) {

  if (pHandler != nullptr)
    return;

#if defined(Q_OS_WIN32)
  std::wstring pathAsStr(report_minidumps_dirpath.begin(),
                         report_minidumps_dirpath.end());
  pHandler =
      new google_breakpad::ExceptionHandler(pathAsStr,
                                            /*FilterCallback*/ 0, dumpCallback,
                                            /*context*/
                                            nullptr, true);
// throw TODOException(
//    "CrashHandlerPrivate::initCrashHandlerPrivate -- no OS_WIN support")
#elif defined(Q_OS_LINUX)
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
#elif defined(Q_OS_MAC)
  throw TODOException(
      "CrashHandlerPrivate::initCrashHandlerPrivate -- no OS_MAC support");
#endif
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
};
