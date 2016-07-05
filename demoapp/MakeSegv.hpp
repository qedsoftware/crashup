#pragma once

#include <QException>

#if defined MAKESEGV_COMPILE
#define COMMON_DLLSPEC Q_DECL_EXPORT
#else
#define COMMON_DLLSPEC Q_DECL_IMPORT
#endif

class COMMON_DLLSPEC MakeSegvException : public QException {
public:
  void raise() const;
  MakeSegvException *clone() const;
};

COMMON_DLLSPEC void make_segfault();
COMMON_DLLSPEC void make_cxxexception();
COMMON_DLLSPEC void thread_make_segfault();
COMMON_DLLSPEC void thread_make_cxxexception();
