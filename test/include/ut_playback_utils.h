// Copyright 2003 Reify, Inc.

#ifndef _UT_PLAYBACK_UTILS_H_
#define _UT_PLAYBACK_UTILS_H_

#include <rc_unittest.h>

#include <rf_playback_utils.h>

class UT_PlaybackUtils : public rcUnitTest {
 public:
 UT_PlaybackUtils();
  ~UT_PlaybackUtils();

  virtual uint32 run();

 private:
  void testGenerateTimeline();
};

#endif // _UT_PLAYBACK_UTILS_H_
