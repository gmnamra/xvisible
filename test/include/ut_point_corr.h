/*
 *      ut_point_corr.h 06/05/2003
 *
 * Copyright (c) 2003 Reify Corp. All rights reserved.
 */

#ifndef __UT_POINT_CORR_H
#define __UT_POINT_CORR_H

#include <rc_unittest.h>
#include <rc_point_corr.h>

class UT_point_corr : public rcUnitTest {
 public:

  UT_point_corr();
  ~UT_point_corr();

  virtual uint32 run();

 private:
  
  void test3by3Space();
  void time3by3Space();
  void test5by5Space();
  void time5by5Space();
  void testSpace();
  void timeSpace();
};

#endif /* __UT_POINT_CORR_H */
