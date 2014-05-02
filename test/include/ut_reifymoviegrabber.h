// Copyright 2003 Reify, Inc.

#ifndef _UT_REIFYMOVIEGRABBER_H_
#define _UT_REIFYMOVIEGRABBER_H_

#include "rc_unittest.h"
#include "rc_reifymoviegrabber.h"

class UT_ReifyMovieGrabber : public rcUnitTest {
 public:
  UT_ReifyMovieGrabber(std::string movieFileName);
  ~UT_ReifyMovieGrabber();

  virtual uint32 run();

 private:
  void simpleTest();
};

#endif // _UT_REIFYMOVIEGRABBER_H_
