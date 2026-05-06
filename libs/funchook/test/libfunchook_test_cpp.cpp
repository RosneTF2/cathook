/*
/^-----^\   data: 2026-04-30
V  o o  V  plik: libs/funchook/test/libfunchook_test_cpp.cpp
 |  Y  |   autor: pupnoodle
  \ Q /
  / - \
  |    \
  |     \     )
  || (___\====
*/

#include "libfunchook_test.h"

TestCpp::TestCpp() {
  m_ = 0;
}

long TestCpp::call(long a, long b) {
  return m_ + a + b;
}
