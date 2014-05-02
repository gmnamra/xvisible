/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2003/09/16 16:27:51  proberts
 *Added new affine support, interpolator interfaces and implementations, and movie-save-from-rcWindows classes
 *
 *Revision 1.2  2003/08/25 12:14:03  arman
 *fixed warnings
 *
 *Revision 1.1  2003/08/22 19:47:03  arman
 *unit test for affine rectangle
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_AFFINE_H
#define __UT_AFFINE_H


#include "rc_unittest.h"
#include "rc_affine.h"

class UT_affine : public rcUnitTest {
 public:

  UT_affine();
  ~UT_affine();

  virtual uint32 run();

 private:
  void basic();
  void getpixelloc();
  void window();
};

#endif /* __UT_AFFINE_H */
