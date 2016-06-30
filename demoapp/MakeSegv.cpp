#define MAKESEGV_COMPILE
#include "MakeSegv.hpp"

#include <cstdio>

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
