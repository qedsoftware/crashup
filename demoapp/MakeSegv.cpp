#define MAKESEGV_COMPILE
#include "MakeSegv.hpp"

#include <cstdio>
#include <thread>

void MakeSegvException::raise() const { throw * this; }
MakeSegvException *MakeSegvException::clone() const {
  return new MakeSegvException(*this);
}

void make_segfault() {
  int *invalid_address = (int *)7;
  *invalid_address = 42;
}

void make_cxxexception() {
  MakeSegvException *e = new MakeSegvException();
  e->raise();
}

void thread_make_segfault() {
  std::thread th(make_segfault);
  th.join();
}

void thread_make_cxxexception() {
  std::thread th(make_cxxexception);
  th.join();
}
