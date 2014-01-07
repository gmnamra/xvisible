/*
 *      ut_kineticimage.h 06/05/2003
 *
 * Copyright (c) 2003 Reify Corp. All rights reserved.
 */

#ifndef __UT_KINETICIMAGE_H
#define __UT_KINETICIMAGE_H

#include <rc_unittest.h>
#include <rc_kineticimage.h>

//#define RUN_GEN

class UT_kineticimage : public rcUnitTest {
 public:

  UT_kineticimage();
  ~UT_kineticimage();

  virtual uint32 run();

 private:
  
  void testVarianceGenerator();
  void testStdDevGenerator();
  void testVelEntropyGenerator();
  void testAccelEntropyGenerator();
  void testThetaEntropyGenerator();
  void testEightBitImageGenerator();
  void testVBigDspRealFFT();

#ifdef RUN_GEN
  void runGenerator();
#endif
  char* _mName;
};

#endif /* __UT_KINETICIMAGE_H */
