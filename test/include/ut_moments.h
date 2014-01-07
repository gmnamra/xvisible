/*
 *      ut_moments.h 06/05/2003
 *
 * Copyright (c) 2003 Reify Corp. All rights reserved.
 */

#ifndef __UT_MOMENTS_H
#define __UT_MOMENTS_H

#include <rc_unittest.h>
#include <rc_moments.h>

class UT_moments : public rcUnitTest {
 public:

  UT_moments();
  ~UT_moments();

  virtual uint32 run();

 private:
  
  void testMomentGenerator();
  void testAutoCorr3by3Point();
  void timeAutoCorr3by3Point();
  void testAutoCorr3by3Line();
  void timeAutoCorr3by3Line();
  void testAutoCorr5by5Point();
  void timeAutoCorr5by5Point();
  void testAutoCorr5by5Line();
  void timeAutoCorr5by5Line();
  void testAutoCorrPoint();
  void timeAutoCorrPoint();
  void testAutoCorrGeneralAnd16 ();
};

#endif /* __UT_MOMENTS_H */
